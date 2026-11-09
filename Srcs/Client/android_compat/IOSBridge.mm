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
    void Java_com_metin2_client_NativeLib_init(void* env, void* obj, void* assetManager, int width, int height);
    void Java_com_metin2_client_NativeLib_render(void* env, void* obj);
    void Java_com_metin2_client_NativeLib_multiTouchEvent(void* env, void* obj, int pointerId, int action, float x, float y);
    void Java_com_metin2_client_NativeLib_keyEvent(void* env, void* obj, int keyCode, int unicodeChar);
}

static UITextField* s_hiddenTextField = nil;
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
        kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst // BGRA8888 in memory
    );
    CGColorSpaceRelease(cs);
    if (!ctx) {
        CGImageRelease(img);
        return false;
    }

    CGContextDrawImage(ctx, CGRectMake(0, 0, w, h), img);
    CGContextRelease(ctx);
    CGImageRelease(img);
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

        // Flip Y for UIKit text drawing inside CoreGraphics context
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
@interface Metin2IOSViewController : GLKViewController <UITextFieldDelegate>
@property (strong, nonatomic) EAGLContext* context;
@end

@implementation Metin2IOSViewController {
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
    self.preferredFramesPerSecond = 60;

    s_hiddenTextField = [[UITextField alloc] initWithFrame:CGRectMake(-100, -100, 10, 10)];
    s_hiddenTextField.delegate = self;
    s_hiddenTextField.autocorrectionType = UITextAutocorrectionTypeNo;
    s_hiddenTextField.autocapitalizationType = UITextAutocapitalizationTypeNone;
    [self.view addSubview:s_hiddenTextField];
}

- (void)glkView:(GLKView*)view drawInRect:(CGRect)rect {
    [EAGLContext setCurrentContext:self.context];
    if (!_engineInitialized) {
        CGFloat scale = [[UIScreen mainScreen] nativeScale];
        int physW = (int)(view.bounds.size.width * scale + 0.5);
        int physH = (int)(view.bounds.size.height * scale + 0.5);
        if (physW > 0 && physH > 0) {
            _engineInitialized = true;
            Java_com_metin2_client_NativeLib_init(nullptr, nullptr, nullptr, physW, physH);
        }
        return;
    }
    Java_com_metin2_client_NativeLib_render(nullptr, nullptr);
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
        int action = (prevCount == 0) ? 0 : 5; // 0 = ACTION_DOWN, 5 = ACTION_POINTER_DOWN
        Java_com_metin2_client_NativeLib_multiTouchEvent(nullptr, nullptr, pid, action, (float)(pt.x * scale), (float)(pt.y * scale));
    }
}

- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    CGFloat scale = [[UIScreen mainScreen] nativeScale];
    for (UITouch* t in touches) {
        int pid = [self slotForTouch:t allocate:false];
        CGPoint pt = [t locationInView:self.view];
        Java_com_metin2_client_NativeLib_multiTouchEvent(nullptr, nullptr, pid, 2, (float)(pt.x * scale), (float)(pt.y * scale));
    }
}

- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    CGFloat scale = [[UIScreen mainScreen] nativeScale];
    for (UITouch* t in touches) {
        int pid = [self slotForTouch:t allocate:false];
        _touchSlots[pid] = nil;
        int remCount = [self activeTouchCount];
        CGPoint pt = [t locationInView:self.view];
        int action = (remCount == 0) ? 1 : 6; // 1 = ACTION_UP, 6 = ACTION_POINTER_UP
        Java_com_metin2_client_NativeLib_multiTouchEvent(nullptr, nullptr, pid, action, (float)(pt.x * scale), (float)(pt.y * scale));
    }
}

- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    for (int i = 0; i < 4; ++i) _touchSlots[i] = nil;
    Java_com_metin2_client_NativeLib_multiTouchEvent(nullptr, nullptr, 0, 3, 0.0f, 0.0f);
}

- (BOOL)textField:(UITextField*)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString*)string {
    if (string.length == 0) {
        Java_com_metin2_client_NativeLib_keyEvent(nullptr, nullptr, 67, 0); // Backspace
    } else {
        for (NSUInteger i = 0; i < string.length; ++i) {
            unichar ch = [string characterAtIndex:i];
            Java_com_metin2_client_NativeLib_keyEvent(nullptr, nullptr, 0, (int)ch);
        }
    }
    return NO;
}

- (BOOL)textFieldShouldReturn:(UITextField*)textField {
    Java_com_metin2_client_NativeLib_keyEvent(nullptr, nullptr, 66, 13); // Return
    [textField resignFirstResponder];
    return YES;
}
@end
@interface Metin2AppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow* window;
@end

@implementation Metin2AppDelegate
- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
    [application setIdleTimerDisabled:YES]; // Prevent screen sleep while playing
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.window.rootViewController = [[Metin2IOSViewController alloc] init];
    [self.window makeKeyAndVisible];
    return YES;
}
@end

int main(int argc, char* argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([Metin2AppDelegate class]));
    }
}
#endif // __APPLE__