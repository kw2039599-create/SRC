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
    [super deleteBackward];
    Java_com_worldonline_client_NativeLib_keyEvent(nullptr, nullptr, 67, 8);
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
        UIFont* font = [UIFont systemFontOfSize:(CGFloat)pxSize];
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

        CGContextTranslateCTM(ctx, 0.0, 64.0);
        CGContextScaleCTM(ctx, 1.0, -1.0);
        UIGraphicsPushContext(ctx);
        [str drawAtPoint:CGPointMake(0.5, 0.5) withAttributes:attrs];
        UIGraphicsPopContext();
        CGContextRelease(ctx);
        return true;
    }
}

extern "C" void IOSToggleSoftKeyboard(bool show) {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (!s_hiddenTextField) return;
        if (show) [s_hiddenTextField becomeFirstResponder];
        else [s_hiddenTextField resignFirstResponder];
    });
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

extern "C" void IOSPlayAudioFile(const char* diskPath, bool isLoopMusic, float volume) {
    if (!diskPath || !*diskPath) return;
    @autoreleasepool {
        NSString* pathStr = [NSString stringWithUTF8String:diskPath];
        NSURL* url = [NSURL fileURLWithPath:pathStr];
        if (!url) return;

        if (isLoopMusic) {
            if (s_bgmAudioPlayer && s_curBgmPath && [s_curBgmPath isEqualToString:pathStr]) {
                s_bgmAudioPlayer.volume = volume;
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
        } else {
            if (!s_activeSfxPlayers) s_activeSfxPlayers = [[NSMutableArray alloc] init];
            for (NSInteger i = (NSInteger)s_activeSfxPlayers.count - 1; i >= 0; --i) {
                if (!s_activeSfxPlayers[i].isPlaying) {
                    [s_activeSfxPlayers removeObjectAtIndex:i];
                }
            }
            if (s_activeSfxPlayers.count >= 8) return;
            NSError* err = nil;
            AVAudioPlayer* sfx = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&err];
            if (sfx && !err) {
                sfx.volume = volume;
                [sfx play];
                [s_activeSfxPlayers addObject:sfx];
            }
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
    if (string.length == 0) {
        Java_com_worldonline_client_NativeLib_keyEvent(nullptr, nullptr, 67, 8);
    } else {
        for (NSUInteger i = 0; i < string.length; ++i) {
            unichar ch = [string characterAtIndex:i];
            Java_com_worldonline_client_NativeLib_keyEvent(nullptr, nullptr, 0, (int)ch);
        }
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
// World Online iOS Patcher (Fullscreen Looping trailer.mp4 + Arabic UI)
// ============================================================================
static UIColor* WorldOnlineColorHex(unsigned int rgb) {
    return [UIColor colorWithRed:((rgb >> 16) & 0xFF) / 255.0f
                           green:((rgb >> 8) & 0xFF) / 255.0f
                            blue:(rgb & 0xFF) / 255.0f
                           alpha:1.0f];
}
#define Metin2ColorHex WorldOnlineColorHex

static NSString* const kPatchBaseUrl = @"https://pub-9d9b6f14e1f345c98cb2d231a3471077.r2.dev/";
static NSString* const kPatchCachePrefKey = @"worldonline_patch_cache_v1";
static NSString* const kTrailerFileName = @"trailer.mp4";

@interface WorldOnlinePatcherViewController : UIViewController
@property (nonatomic, strong) AVPlayer* videoPlayer;
@property (nonatomic, strong) AVPlayerLayer* videoLayer;
@property (nonatomic, strong) CAGradientLayer* vignetteLayer;
@property (nonatomic, strong) UILabel* statusLabel;
@property (nonatomic, strong) UILabel* percentLabel;
@property (nonatomic, strong) UILabel* detailLabel;
@property (nonatomic, strong) UIProgressView* progressView;
@property (nonatomic, strong) UIButton* retryButton;
@property (nonatomic, assign) BOOL isWorkerRunning;
@end

@implementation WorldOnlinePatcherViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [UIApplication sharedApplication].idleTimerDisabled = YES;
    self.view.backgroundColor = [UIColor blackColor];

    // 1. Setup Fullscreen Video Layer (ResizeAspectFill = Zero Black Bars)
    [self setupBackgroundVideo];

    // 2. Subtle Top & Bottom Cinematic Vignette Overlay
    self.vignetteLayer = [CAGradientLayer layer];
    self.vignetteLayer.frame = self.view.bounds;
    self.vignetteLayer.colors = @[
        (id)[UIColor colorWithRed:0.015f green:0.023f blue:0.039f alpha:0.55f].CGColor,
        (id)[UIColor clearColor].CGColor,
        (id)[UIColor colorWithRed:0.015f green:0.023f blue:0.039f alpha:0.15f].CGColor,
        (id)[UIColor colorWithRed:0.020f green:0.031f blue:0.055f alpha:0.85f].CGColor
    ];
    self.vignetteLayer.locations = @[@0.0, @0.25, @0.65, @1.0];
    [self.view.layer addSublayer:self.vignetteLayer];

    // 3. Top Header Branding ("WORLD ONLINE")
    UILabel* title = [[UILabel alloc] init];
    title.text = @"WORLD ONLINE";
    title.textColor = Metin2ColorHex(0xFFF0B8);
    UIFontDescriptor* serifDesc = [[UIFont systemFontOfSize:28.0f weight:UIFontWeightBold].fontDescriptor fontDescriptorWithDesign:UIFontDescriptorSystemDesignSerif];
    title.font = serifDesc ? [UIFont fontWithDescriptor:serifDesc size:28.0f] : [UIFont boldSystemFontOfSize:28.0f];
    title.textAlignment = NSTextAlignmentCenter;
    title.layer.shadowColor = [UIColor blackColor].CGColor;
    title.layer.shadowRadius = 8.0f;
    title.layer.shadowOpacity = 0.9f;
    title.layer.shadowOffset = CGSizeMake(0.0f, 3.0f);
    title.translatesAutoresizingMaskIntoConstraints = NO;
    [self.view addSubview:title];

    UIView* goldLine = [[UIView alloc] init];
    goldLine.backgroundColor = Metin2ColorHex(0xD4AF37);
    goldLine.layer.cornerRadius = 1.0f;
    goldLine.translatesAutoresizingMaskIntoConstraints = NO;
    [self.view addSubview:goldLine];

    // 4. Sleek Glassmorphic Bottom Patcher Dock
    UIView* dock = [[UIView alloc] init];
    dock.backgroundColor = [UIColor colorWithRed:0.047f green:0.071f blue:0.110f alpha:0.84f];
    dock.layer.borderColor = [UIColor colorWithRed:0.831f green:0.686f blue:0.216f alpha:0.65f].CGColor;
    dock.layer.borderWidth = 1.0f;
    dock.layer.cornerRadius = 12.0f;
    dock.translatesAutoresizingMaskIntoConstraints = NO;
    [self.view addSubview:dock];

    self.statusLabel = [[UILabel alloc] init];
    self.statusLabel.text = @"جاري الاتصال بخادم التحديث...";
    self.statusLabel.textColor = Metin2ColorHex(0xF5F7FA);
    self.statusLabel.font = [UIFont boldSystemFontOfSize:13.0f];
    self.statusLabel.textAlignment = NSTextAlignmentRight;
    self.statusLabel.translatesAutoresizingMaskIntoConstraints = NO;
    [dock addSubview:self.statusLabel];

    self.percentLabel = [[UILabel alloc] init];
    self.percentLabel.text = @"0.0%";
    self.percentLabel.textColor = Metin2ColorHex(0xFFD54F);
    self.percentLabel.font = [UIFont boldSystemFontOfSize:14.0f];
    self.percentLabel.textAlignment = NSTextAlignmentLeft;
    self.percentLabel.translatesAutoresizingMaskIntoConstraints = NO;
    [dock addSubview:self.percentLabel];

    self.progressView = [[UIProgressView alloc] initWithProgressViewStyle:UIProgressViewStyleDefault];
    self.progressView.progressTintColor = Metin2ColorHex(0xFFD54F);
    self.progressView.trackTintColor = Metin2ColorHex(0x05070B);
    self.progressView.layer.cornerRadius = 5.0f;
    self.progressView.clipsToBounds = YES;
    self.progressView.progress = 0.0f;
    self.progressView.translatesAutoresizingMaskIntoConstraints = NO;
    [dock addSubview:self.progressView];

    self.detailLabel = [[UILabel alloc] init];
    self.detailLabel.text = @"جاري تجهيز World Online...";
    self.detailLabel.textColor = Metin2ColorHex(0xB0BEC5);
    self.detailLabel.font = [UIFont systemFontOfSize:11.0f];
    self.detailLabel.textAlignment = NSTextAlignmentRight;
    self.detailLabel.translatesAutoresizingMaskIntoConstraints = NO;
    [dock addSubview:self.detailLabel];

    self.retryButton = [UIButton buttonWithType:UIButtonTypeSystem];
    [self.retryButton setTitle:@"إعادة المحاولة" forState:UIControlStateNormal];
    [self.retryButton setTitleColor:Metin2ColorHex(0x0B0F17) forState:UIControlStateNormal];
    self.retryButton.titleLabel.font = [UIFont boldSystemFontOfSize:12.0f];
    self.retryButton.backgroundColor = Metin2ColorHex(0xDAA520);
    self.retryButton.layer.cornerRadius = 6.0f;
    self.retryButton.hidden = YES;
    self.retryButton.translatesAutoresizingMaskIntoConstraints = NO;
    [self.retryButton addTarget:self action:@selector(onRetryTap) forControlEvents:UIControlEventTouchUpInside];
    [dock addSubview:self.retryButton];

    [NSLayoutConstraint activateConstraints:@[
        [title.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor constant:16.0f],
        [title.centerXAnchor constraintEqualToAnchor:self.view.centerXAnchor],
        [goldLine.topAnchor constraintEqualToAnchor:title.bottomAnchor constant:4.0f],
        [goldLine.centerXAnchor constraintEqualToAnchor:self.view.centerXAnchor],
        [goldLine.widthAnchor constraintEqualToConstant:220.0f],
        [goldLine.heightAnchor constraintEqualToConstant:2.0f],

        [dock.leadingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.leadingAnchor constant:32.0f],
        [dock.trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor constant:-32.0f],
        [dock.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor constant:-16.0f],

        [self.percentLabel.topAnchor constraintEqualToAnchor:dock.topAnchor constant:14.0f],
        [self.percentLabel.leadingAnchor constraintEqualToAnchor:dock.leadingAnchor constant:20.0f],

        [self.statusLabel.centerYAnchor constraintEqualToAnchor:self.percentLabel.centerYAnchor],
        [self.statusLabel.leadingAnchor constraintEqualToAnchor:self.percentLabel.trailingAnchor constant:12.0f],
        [self.statusLabel.trailingAnchor constraintEqualToAnchor:dock.trailingAnchor constant:-20.0f],

        [self.progressView.topAnchor constraintEqualToAnchor:self.percentLabel.bottomAnchor constant:8.0f],
        [self.progressView.leadingAnchor constraintEqualToAnchor:dock.leadingAnchor constant:20.0f],
        [self.progressView.trailingAnchor constraintEqualToAnchor:dock.trailingAnchor constant:-20.0f],
        [self.progressView.heightAnchor constraintEqualToConstant:10.0f],

        [self.detailLabel.topAnchor constraintEqualToAnchor:self.progressView.bottomAnchor constant:8.0f],
        [self.detailLabel.trailingAnchor constraintEqualToAnchor:dock.trailingAnchor constant:-20.0f],
        [self.detailLabel.leadingAnchor constraintEqualToAnchor:self.retryButton.trailingAnchor constant:12.0f],
        [self.detailLabel.bottomAnchor constraintEqualToAnchor:dock.bottomAnchor constant:-14.0f],

        [self.retryButton.centerYAnchor constraintEqualToAnchor:self.detailLabel.centerYAnchor],
        [self.retryButton.leadingAnchor constraintEqualToAnchor:dock.leadingAnchor constant:20.0f],
        [self.retryButton.widthAnchor constraintEqualToConstant:115.0f],
        [self.retryButton.heightAnchor constraintEqualToConstant:30.0f]
    ]];

    [self startPatchCheck];
}

- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    if (self.videoLayer) self.videoLayer.frame = self.view.bounds;
    if (self.vignetteLayer) self.vignetteLayer.frame = self.view.bounds;
    [CATransaction commit];
}

- (void)setupBackgroundVideo {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (self.videoPlayer) return;

        NSURL* videoURL = nil;
        // 1. Instant First-Boot: Load bundled trailer.mp4 from inside M2WorldOnline.app
        NSString* bundlePath = [[NSBundle mainBundle] pathForResource:@"trailer" ofType:@"mp4"];
        if (bundlePath && [[NSFileManager defaultManager] fileExistsAtPath:bundlePath]) {
            videoURL = [NSURL fileURLWithPath:bundlePath];
        }

        // 2. Fallback: Check Documents/trailer.mp4
        if (!videoURL) {
            NSString* docs = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
            NSString* localTrailer = [docs stringByAppendingPathComponent:kTrailerFileName];
            if ([[NSFileManager defaultManager] fileExistsAtPath:localTrailer]) {
                videoURL = [NSURL fileURLWithPath:localTrailer];
            }
        }

        // 3. Fallback: Stream from Patch Server
        if (!videoURL) {
            videoURL = [NSURL URLWithString:[kPatchBaseUrl stringByAppendingString:kTrailerFileName]];
        }
        if (!videoURL) return;

        AVAudioSession* session = [AVAudioSession sharedInstance];
        [session setCategory:AVAudioSessionCategoryAmbient error:nil];
        [session setActive:YES error:nil];

        AVPlayerItem* item = [AVPlayerItem playerItemWithURL:videoURL];
        self.videoPlayer = [AVPlayer playerWithPlayerItem:item];
        self.videoPlayer.actionAtItemEnd = AVPlayerActionAtItemEndNone;

        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(onVideoDidReachEnd:)
                                                     name:AVPlayerItemDidPlayToEndTimeNotification
                                                   object:item];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(onAppWillEnterForeground:)
                                                     name:UIApplicationWillEnterForegroundNotification
                                                   object:nil];

        self.videoLayer = [AVPlayerLayer playerLayerWithPlayer:self.videoPlayer];
        self.videoLayer.frame = self.view.bounds;
        self.videoLayer.videoGravity = AVLayerVideoGravityResizeAspectFill;
        [self.view.layer insertSublayer:self.videoLayer atIndex:0];

        [self.videoPlayer play];
    });
}

- (void)onVideoDidReachEnd:(NSNotification*)notification {
    AVPlayerItem* item = notification.object;
    [item seekToTime:kCMTimeZero completionHandler:nil];
    [self.videoPlayer play];
}

- (void)onAppWillEnterForeground:(NSNotification*)notification {
    if (self.videoPlayer) {
        [self.videoPlayer play];
    }
}

- (void)stopAndReleaseVideo {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    if (self.videoPlayer) {
        [self.videoPlayer pause];
        self.videoPlayer = nil;
    }
    if (self.videoLayer) {
        [self.videoLayer removeFromSuperlayer];
        self.videoLayer = nil;
    }
}

- (void)dealloc {
    [self stopAndReleaseVideo];
}

- (void)onRetryTap {
    self.retryButton.hidden = YES;
    [self startPatchCheck];
}

- (void)updateUIStatus:(NSString*)s detail:(NSString*)d progress:(float)p showRetry:(BOOL)r {
    dispatch_async(dispatch_get_main_queue(), ^{
        self.statusLabel.text = s;
        self.detailLabel.text = d;
        if (p >= 0.0f) {
            float clamped = fmaxf(0.0f, fminf(1.0f, p));
            self.progressView.progress = clamped;
            self.percentLabel.text = [NSString stringWithFormat:@"%.1f%%", clamped * 100.0f];
        }
        self.retryButton.hidden = !r;
    });
}

- (void)startPatchCheck {
    if (self.isWorkerRunning) return;
    self.isWorkerRunning = YES;
    [self updateUIStatus:@"جاري الاتصال بخادم التحديث..." detail:@"جاري تجهيز World Online..." progress:0.0f showRetry:NO];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [self runPatchWorker];
        self.isWorkerRunning = NO;
    });
}

- (NSString*)computeMd5:(NSString*)path {
    NSInputStream* st = [NSInputStream inputStreamWithFileAtPath:path];
    if (!st) return @"";
    [st open];
    CC_MD5_CTX c;
    CC_MD5_Init(&c);
    uint8_t b[131072];
    while ([st hasBytesAvailable]) {
        NSInteger n = [st read:b maxLength:sizeof(b)];
        if (n <= 0) break;
        CC_MD5_Update(&c, b, (CC_LONG)n);
    }
    [st close];
    unsigned char dg[CC_MD5_DIGEST_LENGTH];
    CC_MD5_Final(dg, &c);
    NSMutableString* h = [NSMutableString stringWithCapacity:32];
    for (int i = 0; i < CC_MD5_DIGEST_LENGTH; ++i) [h appendFormat:@"%02x", dg[i]];
    return h;
}

- (NSData*)fetchManifest:(NSString*)u {
    for (int a = 1; a <= 4; ++a) {
        if (a > 1) {
            [self updateUIStatus:[NSString stringWithFormat:@"جاري الاتصال بخادم التحديث... (محاولة %d/4)", a]
                          detail:@"إعادة محاولة الاتصال..."
                        progress:0.0f
                       showRetry:NO];
        }
        dispatch_semaphore_t sem = dispatch_semaphore_create(0);
        __block NSData* outData = nil;
        NSMutableURLRequest* req = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:u]
                                                           cachePolicy:NSURLRequestReloadIgnoringLocalCacheData
                                                       timeoutInterval:10.0];
        [req setValue:@"no-cache" forHTTPHeaderField:@"Cache-Control"];
        [[[NSURLSession sharedSession] dataTaskWithRequest:req completionHandler:^(NSData* d, NSURLResponse* r, NSError* e){
            if (!e && [r isKindOfClass:[NSHTTPURLResponse class]] && ((NSHTTPURLResponse*)r).statusCode == 200 && d && d.length > 0)
                outData = [[NSData alloc] initWithData:d];
            dispatch_semaphore_signal(sem);
        }] resume];
        dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
        if (outData) return outData;
        [NSThread sleepForTimeInterval:0.6];
    }
    return nil;
}

- (BOOL)downloadEntry:(NSDictionary*)entry toPath:(NSString*)target base:(long long)base total:(long long)total idx:(int)idx count:(int)count err:(NSString**)err {
    NSString* rel = entry[@"path"];
    NSString* expMd5 = [entry[@"md5"] lowercaseString];
    NSString* tmp = [target stringByAppendingString:@".tmp"];
    NSFileManager* fm = [NSFileManager defaultManager];
    NSTimeInterval t0 = [NSDate timeIntervalSinceReferenceDate];

    for (int a = 1; a <= 4; ++a) {
        [fm removeItemAtPath:tmp error:nil];
        NSURLSessionConfiguration* cfg = [NSURLSessionConfiguration ephemeralSessionConfiguration];
        cfg.timeoutIntervalForRequest = 25.0;
        cfg.timeoutIntervalForResource = 7200.0;
        NSURLSession* sess = [NSURLSession sessionWithConfiguration:cfg];
        NSMutableURLRequest* req = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:[kPatchBaseUrl stringByAppendingString:[rel stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet URLPathAllowedCharacterSet]]]]
                                                           cachePolicy:NSURLRequestReloadIgnoringLocalCacheData
                                                       timeoutInterval:25.0];
        dispatch_semaphore_t sem = dispatch_semaphore_create(0);
        __block BOOL ok = NO;
        NSURLSessionDownloadTask* task = [sess downloadTaskWithRequest:req completionHandler:^(NSURL* loc, NSURLResponse* r, NSError* e){
            if (!e && loc && ((NSHTTPURLResponse*)r).statusCode == 200) {
                [fm removeItemAtPath:tmp error:nil];
                ok = [fm moveItemAtURL:loc toURL:[NSURL fileURLWithPath:tmp] error:nil];
            }
            dispatch_semaphore_signal(sem);
        }];
        [task resume];
        while (dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 80000000LL)) != 0) {
            long long cur = base + task.countOfBytesReceived;
            double dt = MAX(0.1, [NSDate timeIntervalSinceReferenceDate] - t0);
            double spd = (cur / 1048576.0) / dt;
            float prg = total > 0 ? (float)((double)cur / (double)total) : 1.0f;
            [self updateUIStatus:[NSString stringWithFormat:@"جاري تحميل التحديثات (%d/%d)", idx, count]
                          detail:[NSString stringWithFormat:@"%@ • %.1f / %.1f MB (%.2f MB/s)", rel, cur / 1048576.0, total / 1048576.0, spd]
                        progress:prg
                       showRetry:NO];
        }
        [sess finishTasksAndInvalidate];
        if (!ok) {
            if (a < 4) {
                [self updateUIStatus:[NSString stringWithFormat:@"جاري إعادة الاتصال (%d/4): %@", a, rel]
                              detail:@"استئناف التحميل..."
                            progress:-1.0f
                           showRetry:NO];
                [NSThread sleepForTimeInterval:0.8];
            }
            continue;
        }
        if (![[[self computeMd5:tmp] lowercaseString] isEqualToString:expMd5]) {
            [fm removeItemAtPath:tmp error:nil];
            if (a == 4 && err) *err = [NSString stringWithFormat:@"خطأ في التحقق من الملف: %@", rel];
            continue;
        }
        [fm removeItemAtPath:target error:nil];
        if (![fm moveItemAtPath:tmp toPath:target error:nil]) {
            if (err) *err = [NSString stringWithFormat:@"فشل حفظ الملف: %@", rel];
            return NO;
        }
        return YES;
    }
    if (err && !*err) *err = [NSString stringWithFormat:@"فشل تحميل الملف: %@", rel];
    return NO;
}

- (void)runPatchWorker {
    NSString* base = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
    NSFileManager* fm = [NSFileManager defaultManager];
    for (NSString* d in @[@"UserData", @"UserData/environment", @"shop", @"temp", @"upload", @"mark"]) {
        [fm createDirectoryAtPath:[base stringByAppendingPathComponent:d] withIntermediateDirectories:YES attributes:nil error:nil];
    }

    NSData* mData = [self fetchManifest:[NSString stringWithFormat:@"%@manifest.json?t=%lld", kPatchBaseUrl, (long long)([[NSDate date] timeIntervalSince1970] * 1000)]];
    if (!mData) {
        [self updateUIStatus:@"خطأ في التحديث: انتهت مهلة الاتصال بالخادم" detail:@"اضغط على إعادة المحاولة للمتابعة." progress:0.0f showRetry:YES];
        return;
    }

    NSDictionary* root = [NSJSONSerialization JSONObjectWithData:mData options:0 error:nil];
    NSArray* files = root[@"files"];
    if (![files isKindOfClass:[NSArray class]]) {
        [self updateUIStatus:@"خطأ في التحديث: بيانات التحديث غير صالحة" detail:@"اضغط على إعادة المحاولة للمتابعة." progress:0.0f showRetry:YES];
        return;
    }

    NSUserDefaults* def = [NSUserDefaults standardUserDefaults];
    NSMutableDictionary* cache = [[def dictionaryForKey:kPatchCachePrefKey] mutableCopy] ?: [NSMutableDictionary dictionary];
    NSMutableArray* todo = [NSMutableArray array];
    long long totalBytes = 0;
    int cnt = (int)files.count;

    for (int i = 0; i < cnt; ++i) {
        NSDictionary* it = files[i];
        NSString* rel = it[@"path"];
        long long sz = [it[@"size"] longLongValue];
        NSString* md5 = [it[@"md5"] lowercaseString];
        if (i % 8 == 0 || i == cnt - 1) {
            [self updateUIStatus:[NSString stringWithFormat:@"جاري فحص الملفات (%d/%d)...", i + 1, cnt]
                          detail:rel
                        progress:(float)(i + 1) / (float)cnt
                       showRetry:NO];
        }
        NSString* loc = [base stringByAppendingPathComponent:rel];
        BOOL ok = NO;
        if ([fm fileExistsAtPath:loc]) {
            NSDictionary* at = [fm attributesOfItemAtPath:loc error:nil];
            if ([at fileSize] == sz) {
                long long mod = (long long)[[at fileModificationDate] timeIntervalSince1970];
                NSString* cVal = [NSString stringWithFormat:@"%lld:%lld:%@", sz, mod, md5];
                if ([cache[rel] isEqualToString:cVal]) {
                    ok = YES;
                } else if ([[[self computeMd5:loc] lowercaseString] isEqualToString:md5]) {
                    cache[rel] = cVal;
                    ok = YES;
                }
            }
        }
        if (!ok) {
            [todo addObject:it];
            totalBytes += sz;
        }
    }

    [def setObject:cache forKey:kPatchCachePrefKey];
    [def synchronize];

    long long doneBytes = 0;
    for (int i = 0; i < (int)todo.count; ++i) {
        NSDictionary* it = todo[i];
        NSString* rel = it[@"path"];
        NSString* dst = [base stringByAppendingPathComponent:rel];
        [fm createDirectoryAtPath:[dst stringByDeletingLastPathComponent] withIntermediateDirectories:YES attributes:nil error:nil];
        NSString* err = nil;
        if (![self downloadEntry:it toPath:dst base:doneBytes total:totalBytes idx:i + 1 count:(int)todo.count err:&err]) {
            [self updateUIStatus:[NSString stringWithFormat:@"خطأ في التحديث: %@", err ?: @"فشل التحميل"]
                          detail:@"اضغط على إعادة المحاولة للمتابعة."
                        progress:0.0f
                       showRetry:YES];
            return;
        }
        doneBytes += [it[@"size"] longLongValue];
        NSDictionary* at = [fm attributesOfItemAtPath:dst error:nil];
        cache[rel] = [NSString stringWithFormat:@"%lld:%lld:%@", [it[@"size"] longLongValue], (long long)[[at fileModificationDate] timeIntervalSince1970], [it[@"md5"] lowercaseString]];
        [def setObject:cache forKey:kPatchCachePrefKey];
    }

    [def synchronize];
    [self updateUIStatus:@"اكتمل التحديث! جاري تشغيل اللعبة..." detail:@"" progress:1.0f showRetry:NO];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 250000000LL), dispatch_get_main_queue(), ^{
        chdir([base UTF8String]);
        setenv("WORLDONLINE_IOS_DOCS_DIR", [base UTF8String], 1);
        NSString* crashPath = [base stringByAppendingPathComponent:@"crash_report.txt"];
        if ([[NSFileManager defaultManager] fileExistsAtPath:crashPath]) {
            NSData* d = [NSData dataWithContentsOfFile:crashPath];
            NSString* rep = [[NSString alloc] initWithData:d encoding:NSUTF8StringEncoding] ?: [[NSString alloc] initWithData:d encoding:NSISOLatin1StringEncoding] ?: @"[Unreadable crash report]";
            UIAlertController* al = [UIAlertController alertControllerWithTitle:@"Crash Detected (Previous Run)" message:rep preferredStyle:UIAlertControllerStyleAlert];
            [al addAction:[UIAlertAction actionWithTitle:@"COPY CRASH REPORT" style:UIAlertActionStyleDefault handler:^(UIAlertAction* a){
                [UIPasteboard generalPasteboard].string = rep;
                self.statusLabel.text = @"تم نسخ تقرير الخطأ! اضغط على إعادة المحاولة لتشغيل اللعبة.";
                [[NSFileManager defaultManager] removeItemAtPath:crashPath error:nil];
                self.retryButton.hidden = NO;
            }]];
            [al addAction:[UIAlertAction actionWithTitle:@"CLEAR & LAUNCH GAME" style:UIAlertActionStyleDestructive handler:^(UIAlertAction* a){
                [[NSFileManager defaultManager] removeItemAtPath:crashPath error:nil];
                [self stopAndReleaseVideo];
                UIWindow* w = [UIApplication sharedApplication].delegate.window ?: [UIApplication sharedApplication].windows.firstObject;
                w.rootViewController = [[WorldOnlineIOSViewController alloc] init];
                [w makeKeyAndVisible];
            }]];
            [self presentViewController:al animated:YES completion:nil];
            return;
        }
        [self stopAndReleaseVideo];
        UIWindow* w = [UIApplication sharedApplication].delegate.window;
        if (!w) w = [UIApplication sharedApplication].windows.firstObject;
        w.rootViewController = [[WorldOnlineIOSViewController alloc] init];
        [w makeKeyAndVisible];
    });
}

- (UIInterfaceOrientationMask)supportedInterfaceOrientations { return UIInterfaceOrientationMaskLandscape; }
- (BOOL)prefersStatusBarHidden { return YES; }
@end

@interface WorldOnlineAppDelegate : UIResponder <UIApplicationDelegate>
@property (nonatomic, strong) UIWindow* window;
@end

@implementation WorldOnlineAppDelegate
- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
    NSString* docs = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
    chdir([docs UTF8String]);
    setenv("WORLDONLINE_IOS_DOCS_DIR", [docs UTF8String], 1);
    self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    self.window.rootViewController = [[WorldOnlinePatcherViewController alloc] init];
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