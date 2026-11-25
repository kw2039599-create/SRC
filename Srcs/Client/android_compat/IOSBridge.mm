#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#import <CommonCrypto/CommonDigest.h>
#ifdef __APPLE__
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#import <OpenGLES/ES3/gl.h>
#import <CoreGraphics/CoreGraphics.h>
#import <ImageIO/ImageIO.h>
#import <AVFoundation/AVFoundation.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <stdint.h>

extern "C" {
    void Java_com_worldonline_client_NativeLib_init(void* env, void* obj, void* assetManager, int width, int height);
    void Java_com_worldonline_client_NativeLib_render(void* env, void* obj);
    void Java_com_worldonline_client_NativeLib_multiTouchEvent(void* env, void* obj, int pointerId, int action, float x, float y);
    void Java_com_worldonline_client_NativeLib_keyEvent(void* env, void* obj, int keyCode, int unicodeChar);
}

@interface WorldOnlineHiddenTextField : UITextField
@end

@implementation WorldOnlineHiddenTextField
- (void)deleteBackward {
    Java_com_worldonline_client_NativeLib_keyEvent(nullptr, nullptr, 67, 8);
    if (self.text.length < 16) {
        self.text = @"                                                                ";
    }
}
@end

static WorldOnlineHiddenTextField* s_hiddenTextField = nil;
static AVAudioPlayer* s_bgmAudioPlayer = nil;
static NSString* s_curBgmPath = nil;
static NSMutableArray<AVAudioPlayer*>* s_activeSfxPlayers = nil;

extern "C" const char* IOSGetDocumentsDirectory() {
    static char s_docPath[1024] = {0};
    if (!s_docPath[0]) {
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString* docDir = [paths firstObject];
        if (docDir) {
            strncpy(s_docPath, [docDir UTF8String], sizeof(s_docPath) - 1);
        } else {
            strcpy(s_docPath, ".");
        }
    }
    return s_docPath;
}

extern "C" bool IOSDecodeImageFromMemory(const void* data, unsigned int size, unsigned int* outW, unsigned int* outH, std::vector<uint32_t>& outBGRA) {
    if (!data || size < 8 || !outW || !outH) return false;
    CFDataRef cfData = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (const UInt8*)data, (CFIndex)size, kCFAllocatorNull);
    if (!cfData) return false;

    CGImageSourceRef src = CGImageSourceCreateWithData(cfData, NULL);
    CFRelease(cfData);
    if (!src) return false;

    CGImageRef img = CGImageSourceCreateImageAtIndex(src, 0, NULL);
    CFRelease(src);
    if (!img) return false;

    size_t w = CGImageGetWidth(img);
    size_t h = CGImageGetHeight(img);
    if (w == 0 || h == 0 || w > 4096 || h > 4096) {
        CGImageRelease(img);
        return false;
    }

    *outW = (unsigned int)w;
    *outH = (unsigned int)h;
    outBGRA.resize(w * h, 0);

    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(
        outBGRA.data(), w, h, 8, w * 4, cs,
        kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst
    );
    CGColorSpaceRelease(cs);
    if (!ctx) {
        CGImageRelease(img);
        return false;
    }

    CGContextDrawImage(ctx, CGRectMake(0, 0, w, h), img);
    CGContextRelease(ctx);
    CGImageRelease(img);

    uint32_t* pixels = outBGRA.data();
    size_t totalPixels = w * h;
    for (size_t i = 0; i < totalPixels; ++i) {
        uint32_t p = pixels[i];
        uint32_t a = (p >> 24) & 0xFF;
        if (a > 0 && a < 255) {
            uint32_t b = (p & 0xFF) * 255 / a;
            uint32_t g = ((p >> 8) & 0xFF) * 255 / a;
            uint32_t r = ((p >> 16) & 0xFF) * 255 / a;
            pixels[i] = (a << 24) | ((r > 255 ? 255 : r) << 16) | ((g > 255 ? 255 : g) << 8) | (b > 255 ? 255 : b);
        }
    }
    return true;
}

extern "C" bool IOSRenderCharGlyph(int pxSize, unsigned short ch, int* outW, int* outH, int* outAdv, uint32_t* outPixels64) {
    if (!outW || !outH || !outAdv || !outPixels64) return false;
    @autoreleasepool {
        unichar uc = (unichar)ch;
        NSString* str = [NSString stringWithCharacters:&uc length:1];
        UIFont* font = [UIFont systemFontOfSize:(CGFloat)pxSize weight:UIFontWeightMedium];
        NSDictionary* attrs = @{
            NSFontAttributeName: font,
            NSForegroundColorAttributeName: [UIColor whiteColor]
        };
        CGSize sz = [str sizeWithAttributes:attrs];
        int adv = (int)ceilf(sz.width);
        int w = adv + 2;
        int h = (int)ceilf(sz.height) + 2;
        if (w < 4) w = 4; else if (w > 64) w = 64;
        if (h < 8) h = 8; else if (h > 64) h = 64;

        *outW = w;
        *outH = h;
        *outAdv = adv;

        memset(outPixels64, 0, 64 * h * sizeof(uint32_t));
        CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
        CGContextRef ctx = CGBitmapContextCreate(
            outPixels64, 64, 64, 8, 64 * 4, cs,
            kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst
        );
        CGColorSpaceRelease(cs);
        if (!ctx) return false;

        CGContextSetAllowsAntialiasing(ctx, true);
        CGContextSetShouldAntialias(ctx, true);
        CGContextSetAllowsFontSmoothing(ctx, false);
        CGContextTranslateCTM(ctx, 0.0, 64.0);
        CGContextScaleCTM(ctx, 1.0, -1.0);
        UIGraphicsPushContext(ctx);
        // Draw on exact integer pixel origin (0.0, 0.0) instead of (0.5, 0.5) to prevent subpixel blur
        [str drawAtPoint:CGPointMake(0.0, 0.0) withAttributes:attrs];
        UIGraphicsPopContext();
        CGContextRelease(ctx);
        return true;
    }
}

extern "C" void IOSToggleSoftKeyboard(bool show) {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (!s_hiddenTextField) return;
        if (show) {
            s_hiddenTextField.text = @"                                                                ";
            [s_hiddenTextField becomeFirstResponder];
        } else {
            [s_hiddenTextField resignFirstResponder];
        }
    });
}

extern "C" void IOSOpenURL(const char* urlStr) {
    if (!urlStr || !*urlStr) return;
    NSString* nsUrl = [NSString stringWithUTF8String:urlStr];
    if (!nsUrl) return;
    dispatch_async(dispatch_get_main_queue(), ^{
        NSURL* url = [NSURL URLWithString:nsUrl];
        if (url) {
            [[UIApplication sharedApplication] openURL:url options:@{} completionHandler:nil];
        }
    });
}

extern "C" void IOSCloseApp() {
    // Guideline 2.5.1 / 2.5.19 compliance: Do not invoke private UIApplication selectors or force-terminate on iOS
    IOSStopMusic();
}

extern "C" void IOSSetMusicVolume(float volume) {
    if (s_bgmAudioPlayer) {
        s_bgmAudioPlayer.volume = (volume < 0.0f) ? 0.0f : ((volume > 1.0f) ? 1.0f : volume);
    }
}

extern "C" void IOSStopMusic() {
    if (s_bgmAudioPlayer) {
        [s_bgmAudioPlayer stop];
        s_bgmAudioPlayer = nil;
        s_curBgmPath = nil;
    }
}

static NSMutableDictionary<NSString*, NSData*>* s_sfxDataCache = nil;

extern "C" void IOSPlayAudioFile3D(const char* diskPath, float volume, float pan, float rate) {
    if (!diskPath || !*diskPath || volume <= 0.001f) return;
    @autoreleasepool {
        NSString* pathStr = [NSString stringWithUTF8String:diskPath];
        if (!pathStr) return;

        if (!s_sfxDataCache) s_sfxDataCache = [[NSMutableDictionary alloc] init];
        NSData* audioData = s_sfxDataCache[pathStr];
        if (!audioData) {
            audioData = [NSData dataWithContentsOfFile:pathStr];
            if (!audioData) return;
            if (s_sfxDataCache.count > 128) [s_sfxDataCache removeAllObjects];
            s_sfxDataCache[pathStr] = audioData;
        }

        if (!s_activeSfxPlayers) s_activeSfxPlayers = [[NSMutableArray alloc] init];
        for (NSInteger i = (NSInteger)s_activeSfxPlayers.count - 1; i >= 0; --i) {
            if (!s_activeSfxPlayers[i].isPlaying) {
                [s_activeSfxPlayers removeObjectAtIndex:i];
            }
        }
        if (s_activeSfxPlayers.count >= 16) return;
        NSError* err = nil;
        AVAudioPlayer* sfx = [[AVAudioPlayer alloc] initWithData:audioData error:&err];
        if (sfx && !err) {
            sfx.volume = (volume > 1.0f) ? 1.0f : volume;
            sfx.pan = (pan < -1.0f) ? -1.0f : ((pan > 1.0f) ? 1.0f : pan);
            if (fabsf(rate - 1.0f) > 0.005f) {
                sfx.enableRate = YES;
                sfx.rate = rate;
            }
            [sfx play];
            [s_activeSfxPlayers addObject:sfx];
        }
    }
}

extern "C" void IOSPlayAudioFile(const char* diskPath, bool isLoopMusic, float volume) {
    if (!diskPath || !*diskPath) return;
    if (!isLoopMusic) {
        IOSPlayAudioFile3D(diskPath, volume, 0.0f, 1.0f);
        return;
    }
    @autoreleasepool {
        NSString* pathStr = [NSString stringWithUTF8String:diskPath];
        NSURL* url = [NSURL fileURLWithPath:pathStr];
        if (!url) return;

        if (s_bgmAudioPlayer && s_curBgmPath && [s_curBgmPath isEqualToString:pathStr]) {
            s_bgmAudioPlayer.volume = volume;
            if (!s_bgmAudioPlayer.isPlaying) [s_bgmAudioPlayer play];
            return;
        }
        IOSStopMusic();
        NSError* err = nil;
        s_bgmAudioPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&err];
        if (s_bgmAudioPlayer && !err) {
            s_bgmAudioPlayer.numberOfLoops = -1;
            s_bgmAudioPlayer.volume = volume;
            [s_bgmAudioPlayer prepareToPlay];
            [s_bgmAudioPlayer play];
            s_curBgmPath = pathStr;
        }
    }
}

// ============================================================================
// iOS GLKViewController Hosting OpenGL ES 3.0 + Multi-Touch Forwarding
// ============================================================================
@interface WorldOnlineIOSViewController : GLKViewController <UITextFieldDelegate>
@property (strong, nonatomic) EAGLContext* context;
@end

@implementation WorldOnlineIOSViewController {
    UITouch* _touchSlots[4];
    bool _engineInitialized;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    for (int i = 0; i < 4; ++i) _touchSlots[i] = nil;
    _engineInitialized = false;

    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    [EAGLContext setCurrentContext:self.context];

    GLKView* view = (GLKView*)self.view;
    view.context = self.context;
    view.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    view.drawableStencilFormat = GLKViewDrawableStencilFormat8;
    view.multipleTouchEnabled = YES;
    view.contentScaleFactor = [[UIScreen mainScreen] nativeScale];
    view.opaque = YES;
    view.layer.opaque = YES;
    self.preferredFramesPerSecond = 60;

    s_hiddenTextField = [[WorldOnlineHiddenTextField alloc] initWithFrame:CGRectMake(-100, -100, 10, 10)];
    s_hiddenTextField.delegate = self;
    s_hiddenTextField.autocorrectionType = UITextAutocorrectionTypeNo;
    s_hiddenTextField.autocapitalizationType = UITextAutocapitalizationTypeNone;
    [self.view addSubview:s_hiddenTextField];
}

- (void)onToggleKeyboard {
    for (UIView* v in self.view.subviews) {
        if ([v isKindOfClass:[UITextField class]]) {
            if ([v isFirstResponder]) [v resignFirstResponder];
            else [v becomeFirstResponder];
            return;
        }
    }
}

- (NSString*)readTailOfFile:(NSString*)p maxBytes:(NSUInteger)m {
    NSFileManager* fm = [NSFileManager defaultManager];
    if (![fm fileExistsAtPath:p]) return @"[File does not exist]";
    NSData* d = [NSData dataWithContentsOfFile:p];
    if (!d || d.length == 0) return @"[File is empty]";
    if (d.length > m) d = [d subdataWithRange:NSMakeRange(d.length - m, m)];
    NSString* s = [[NSString alloc] initWithData:d encoding:NSUTF8StringEncoding];
    if (!s) s = [[NSString alloc] initWithData:d encoding:NSISOLatin1StringEncoding];
    return s ?: @"[Unable to decode]";
}

- (void)onShowLogs {
    NSString* docs = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
    NSString* cr = [self readTailOfFile:[docs stringByAppendingPathComponent:@"crash_report.txt"] maxBytes:8000];
    NSString* se = [self readTailOfFile:[docs stringByAppendingPathComponent:@"syserr.txt"] maxBytes:3000];
    NSString* lg = [self readTailOfFile:[docs stringByAppendingPathComponent:@"log.txt"] maxBytes:3000];
    NSString* fl = [NSString stringWithFormat:@"=== CRASH_REPORT.TXT ===\n%@\n\n=== SYSERR.TXT ===\n%@\n\n=== LOG.TXT ===\n%@\n", cr, se, lg];
    UIAlertController* al = [UIAlertController alertControllerWithTitle:@"Diagnostic & Crash Logs" message:fl preferredStyle:UIAlertControllerStyleAlert];
    [al addAction:[UIAlertAction actionWithTitle:@"COPY LOG" style:UIAlertActionStyleDefault handler:^(UIAlertAction* a){
        [UIPasteboard generalPasteboard].string = fl;
    }]];
    [al addAction:[UIAlertAction actionWithTitle:@"CLEAR CRASH" style:UIAlertActionStyleDestructive handler:^(UIAlertAction* a){
        [[NSFileManager defaultManager] removeItemAtPath:[docs stringByAppendingPathComponent:@"crash_report.txt"] error:nil];
    }]];
    [al addAction:[UIAlertAction actionWithTitle:@"CLOSE" style:UIAlertActionStyleCancel handler:nil]];
    [self presentViewController:al animated:YES completion:nil];
}

- (void)glkView:(GLKView*)view drawInRect:(CGRect)rect {
    [EAGLContext setCurrentContext:self.context];
    if (!_engineInitialized) {
        int physW = (int)view.drawableWidth;
        int physH = (int)view.drawableHeight;
        if (physW <= 0 || physH <= 0) {
            CGFloat scale = [[UIScreen mainScreen] nativeScale];
            physW = (int)(view.bounds.size.width * scale + 0.5);
            physH = (int)(view.bounds.size.height * scale + 0.5);
        }
        if (physW > 0 && physH > 0) {
            _engineInitialized = true;
            Java_com_worldonline_client_NativeLib_init(nullptr, nullptr, nullptr, physW, physH);
        }
        return;
    }
    Java_com_worldonline_client_NativeLib_render(nullptr, nullptr);
}

- (int)slotForTouch:(UITouch*)touch allocate:(bool)alloc {
    for (int i = 0; i < 4; ++i) {
        if (_touchSlots[i] == touch) return i;
    }
    if (alloc) {
        for (int i = 0; i < 4; ++i) {
            if (_touchSlots[i] == nil) {
                _touchSlots[i] = touch;
                return i;
            }
        }
    }
    return 0;
}

- (int)activeTouchCount {
    int c = 0;
    for (int i = 0; i < 4; ++i) if (_touchSlots[i] != nil) ++c;
    return c;
}

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    CGFloat scale = [[UIScreen mainScreen] nativeScale];
    for (UITouch* t in touches) {
        int prevCount = [self activeTouchCount];
        int pid = [self slotForTouch:t allocate:true];
        CGPoint pt = [t locationInView:self.view];
        int action = (prevCount == 0) ? 0 : 5;
        Java_com_worldonline_client_NativeLib_multiTouchEvent(nullptr, nullptr, pid, action, (float)(pt.x * scale), (float)(pt.y * scale));
    }
}

- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    CGFloat scale = [[UIScreen mainScreen] nativeScale];
    for (UITouch* t in touches) {
        int pid = [self slotForTouch:t allocate:false];
        CGPoint pt = [t locationInView:self.view];
        Java_com_worldonline_client_NativeLib_multiTouchEvent(nullptr, nullptr, pid, 2, (float)(pt.x * scale), (float)(pt.y * scale));
    }
}

- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    CGFloat scale = [[UIScreen mainScreen] nativeScale];
    for (UITouch* t in touches) {
        int pid = [self slotForTouch:t allocate:false];
        _touchSlots[pid] = nil;
        int remCount = [self activeTouchCount];
        CGPoint pt = [t locationInView:self.view];
        int action = (remCount == 0) ? 1 : 6;
        Java_com_worldonline_client_NativeLib_multiTouchEvent(nullptr, nullptr, pid, action, (float)(pt.x * scale), (float)(pt.y * scale));
    }
}

- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    for (int i = 0; i < 4; ++i) _touchSlots[i] = nil;
    Java_com_worldonline_client_NativeLib_multiTouchEvent(nullptr, nullptr, 0, 3, 0.0f, 0.0f);
}

- (BOOL)textField:(UITextField*)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString*)string {
    if (string.length == 0 && range.length > 0) {
        for (NSUInteger i = 0; i < range.length; ++i) {
            Java_com_worldonline_client_NativeLib_keyEvent(nullptr, nullptr, 67, 8);
        }
        if (textField.text.length < 16) {
            textField.text = @"                                                                ";
        }
        return NO;
    }
    for (NSUInteger i = 0; i < string.length; ++i) {
        unichar ch = [string characterAtIndex:i];
        Java_com_worldonline_client_NativeLib_keyEvent(nullptr, nullptr, 0, (int)ch);
    }
    return NO;
}

- (BOOL)textFieldShouldReturn:(UITextField*)textField {
    Java_com_worldonline_client_NativeLib_keyEvent(nullptr, nullptr, 66, 13);
    [textField resignFirstResponder];
    return YES;
}
@end

// ============================================================================
// Self-Contained iOS App Bundle Asset Linker (Guideline 2.5.19 Compliant)
// ============================================================================
static void WorldOnlineExcludeFromBackup(NSString* path) {
    if (!path) return;
    NSURL* url = [NSURL fileURLWithPath:path];
    if (url) {
        [url setResourceValue:@YES forKey:NSURLIsExcludedFromBackupKey error:nil];
    }
}

static void WorldOnlineSyncBundledAssetsToDocuments() {
    NSFileManager* fm = [NSFileManager defaultManager];
    NSString* docs = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
    if (!docs) return;
    WorldOnlineExcludeFromBackup(docs);

    // 1. Ensure writable runtime directories exist in Documents
    NSArray<NSString*>* writableDirs = @[@"UserData", @"UserData/environment", @"shop", @"temp", @"upload", @"mark"];
    for (NSString* dir in writableDirs) {
        NSString* subPath = [docs stringByAppendingPathComponent:dir];
        BOOL isDir = NO;
        NSDictionary* attrs = [fm attributesOfItemAtPath:subPath error:nil];
        if (attrs && [attrs[NSFileType] isEqualToString:NSFileTypeSymbolicLink]) {
            [fm removeItemAtPath:subPath error:nil];
        }
        if (![fm fileExistsAtPath:subPath isDirectory:&isDir] || !isDir) {
            [fm createDirectoryAtPath:subPath withIntermediateDirectories:YES attributes:nil error:nil];
        }
        WorldOnlineExcludeFromBackup(subPath);
    }

    // 2. Link or copy bundled GameAssets from WorldOnline.app/GameAssets into Documents
    NSString* bundleAssetsDir = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"GameAssets"];
    BOOL hasBundleAssets = NO;
    if (![fm fileExistsAtPath:bundleAssetsDir isDirectory:&hasBundleAssets] || !hasBundleAssets) {
        return;
    }

    NSArray<NSString*>* items = [fm contentsOfDirectoryAtPath:bundleAssetsDir error:nil];
    for (NSString* item in items) {
        if ([item hasPrefix:@"."]) continue;
        NSString* srcPath = [bundleAssetsDir stringByAppendingPathComponent:item];
        NSString* dstPath = [docs stringByAppendingPathComponent:item];

        // "mark" must stay writable in Documents; copy initial bundled mark files if missing
        if ([[item lowercaseString] isEqualToString:@"mark"]) {
            NSArray<NSString*>* markFiles = [fm contentsOfDirectoryAtPath:srcPath error:nil];
            for (NSString* mf in markFiles) {
                NSString* mSrc = [srcPath stringByAppendingPathComponent:mf];
                NSString* mDst = [dstPath stringByAppendingPathComponent:mf];
                if (![fm fileExistsAtPath:mDst]) {
                    [fm copyItemAtPath:mSrc toPath:mDst error:nil];
                }
            }
            continue;
        }

        BOOL isDir = NO;
        [fm fileExistsAtPath:srcPath isDirectory:&isDir];

        // Small mutable config files (.cfg) are copied once so the engine can modify them
        if (!isDir && [[[item pathExtension] lowercaseString] isEqualToString:@"cfg"]) {
            NSDictionary* dstAttrs = [fm attributesOfItemAtPath:dstPath error:nil];
            if (dstAttrs && [dstAttrs[NSFileType] isEqualToString:NSFileTypeSymbolicLink]) {
                [fm removeItemAtPath:dstPath error:nil];
            }
            if (![fm fileExistsAtPath:dstPath]) {
                [fm copyItemAtPath:srcPath toPath:dstPath error:nil];
                WorldOnlineExcludeFromBackup(dstPath);
            }
            continue;
        }

        // Read-only directories (pack, lib, bgm) and data tables are symlinked to the signed .app bundle
        // Always refresh the symlink on launch because iOS changes the .app bundle UUID across updates
        [fm removeItemAtPath:dstPath error:nil];
        [fm createSymbolicLinkAtPath:dstPath withDestinationPath:srcPath error:nil];
        WorldOnlineExcludeFromBackup(dstPath);
    }
}

@interface WorldOnlineAppDelegate : UIResponder <UIApplicationDelegate>
@property (nonatomic, strong) UIWindow* window;
@end

@implementation WorldOnlineAppDelegate
- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
    [UIApplication sharedApplication].idleTimerDisabled = YES;
    WorldOnlineSyncBundledAssetsToDocuments();

    NSString* docs = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
    chdir([docs UTF8String]);
    setenv("WORLDONLINE_IOS_DOCS_DIR", [docs UTF8String], 1);
    setenv("PYTHONDONTWRITEBYTECODE", "1", 1);

    self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    self.window.rootViewController = [[WorldOnlineIOSViewController alloc] init];
    [self.window makeKeyAndVisible];
    return YES;
}

- (UIInterfaceOrientationMask)application:(UIApplication*)application supportedInterfaceOrientationsForWindow:(UIWindow*)window {
    return UIInterfaceOrientationMaskLandscape;
}
@end

int main(int argc, char* argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([WorldOnlineAppDelegate class]));
    }
}
#endif