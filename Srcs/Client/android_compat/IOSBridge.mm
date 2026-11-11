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
    UIButton* kbBtn=[UIButton buttonWithType:UIButtonTypeSystem]; [kbBtn setTitle:@"⌨" forState:UIControlStateNormal]; [kbBtn setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal]; kbBtn.backgroundColor=[UIColor colorWithWhite:0.0f alpha:0.47f]; kbBtn.layer.cornerRadius=4.0f; kbBtn.translatesAutoresizingMaskIntoConstraints=NO; [kbBtn addTarget:self action:@selector(onToggleKeyboard) forControlEvents:UIControlEventTouchUpInside]; [self.view addSubview:kbBtn];
    UIButton* logBtn=[UIButton buttonWithType:UIButtonTypeSystem]; [logBtn setTitle:@"LOG" forState:UIControlStateNormal]; [logBtn setTitleColor:[UIColor yellowColor] forState:UIControlStateNormal]; logBtn.titleLabel.font=[UIFont boldSystemFontOfSize:11.0f]; logBtn.backgroundColor=[UIColor colorWithWhite:0.0f alpha:0.63f]; logBtn.layer.cornerRadius=4.0f; logBtn.translatesAutoresizingMaskIntoConstraints=NO; [logBtn addTarget:self action:@selector(onShowLogs) forControlEvents:UIControlEventTouchUpInside]; [self.view addSubview:logBtn];
    [NSLayoutConstraint activateConstraints:@[[kbBtn.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor constant:8.0f],[kbBtn.trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor constant:-12.0f],[kbBtn.widthAnchor constraintEqualToConstant:48.0f],[kbBtn.heightAnchor constraintEqualToConstant:38.0f],[logBtn.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor constant:8.0f],[logBtn.trailingAnchor constraintEqualToAnchor:kbBtn.leadingAnchor constant:-8.0f],[logBtn.widthAnchor constraintEqualToConstant:56.0f],[logBtn.heightAnchor constraintEqualToConstant:38.0f]]];
}
- (void)onToggleKeyboard { for(UIView* v in self.view.subviews){ if([v isKindOfClass:[UITextField class]]){ if([v isFirstResponder])[v resignFirstResponder]; else [v becomeFirstResponder]; return; } } }
- (NSString*)readTailOfFile:(NSString*)p maxBytes:(NSUInteger)m { NSFileManager* fm=[NSFileManager defaultManager]; if(![fm fileExistsAtPath:p]) return @"[File does not exist]"; NSData* d=[NSData dataWithContentsOfFile:p]; if(!d||d.length==0) return @"[File is empty]"; if(d.length>m) d=[d subdataWithRange:NSMakeRange(d.length-m,m)]; NSString* s=[[NSString alloc] initWithData:d encoding:NSUTF8StringEncoding]; if(!s) s=[[NSString alloc] initWithData:d encoding:NSISOLatin1StringEncoding]; return s?:@"[Unable to decode]"; }
- (void)onShowLogs { NSString* docs=[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES) firstObject]; NSString* se=[self readTailOfFile:[docs stringByAppendingPathComponent:@"syserr.txt"] maxBytes:4000]; NSString* lg=[self readTailOfFile:[docs stringByAppendingPathComponent:@"log.txt"] maxBytes:4000]; NSString* fl=[NSString stringWithFormat:@"=== DOCS ===\n%@\n\n=== SYSERR.TXT ===\n%@\n\n=== LOG.TXT ===\n%@\n",docs,se,lg]; UIAlertController* al=[UIAlertController alertControllerWithTitle:@"Diagnostic Logs" message:fl preferredStyle:UIAlertControllerStyleAlert]; [al addAction:[UIAlertAction actionWithTitle:@"COPY LOG" style:UIAlertActionStyleDefault handler:^(UIAlertAction* a){[UIPasteboard generalPasteboard].string=fl;}]]; [al addAction:[UIAlertAction actionWithTitle:@"CLOSE" style:UIAlertActionStyleCancel handler:nil]]; [self presentViewController:al animated:YES completion:nil]; }

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
static UIColor* Metin2ColorHex(unsigned int rgb){return [UIColor colorWithRed:((rgb>>16)&0xFF)/255.0f green:((rgb>>8)&0xFF)/255.0f blue:(rgb&0xFF)/255.0f alpha:1.0f];}
static NSString* const kPatchBaseUrl=@"https://pub-9d9b6f14e1f345c98cb2d231a3471077.r2.dev/";
static NSString* const kPatchCachePrefKey=@"metin2_patch_cache_v1";
@interface Metin2PatcherViewController : UIViewController
@property (nonatomic,strong) UILabel* statusLabel;
@property (nonatomic,strong) UILabel* detailLabel;
@property (nonatomic,strong) UIProgressView* progressView;
@property (nonatomic,strong) UIButton* retryButton;
@property (nonatomic,assign) BOOL isWorkerRunning;
@end
@implementation Metin2PatcherViewController
- (void)viewDidLoad {
 [super viewDidLoad]; [UIApplication sharedApplication].idleTimerDisabled=YES; self.view.backgroundColor=Metin2ColorHex(0x0B0F17);
 UIView* card=[[UIView alloc] init]; card.backgroundColor=Metin2ColorHex(0x131B28); card.layer.borderColor=Metin2ColorHex(0xDAA520).CGColor; card.layer.borderWidth=2.0f; card.layer.cornerRadius=6.0f; card.translatesAutoresizingMaskIntoConstraints=NO; [self.view addSubview:card];
 UILabel* title=[[UILabel alloc] init]; title.text=@"M2WORLD ONLINE"; title.textColor=Metin2ColorHex(0xFFE082); title.font=[UIFont boldSystemFontOfSize:22.0f]; title.textAlignment=NSTextAlignmentCenter; title.translatesAutoresizingMaskIntoConstraints=NO; [card addSubview:title];
 self.statusLabel=[[UILabel alloc] init]; self.statusLabel.text=@"Checking for updates..."; self.statusLabel.textColor=[UIColor whiteColor]; self.statusLabel.font=[UIFont systemFontOfSize:14.0f]; self.statusLabel.textAlignment=NSTextAlignmentCenter; self.statusLabel.numberOfLines=2; self.statusLabel.translatesAutoresizingMaskIntoConstraints=NO; [card addSubview:self.statusLabel];
 self.progressView=[[UIProgressView alloc] initWithProgressViewStyle:UIProgressViewStyleDefault]; self.progressView.progressTintColor=Metin2ColorHex(0xDAA520); self.progressView.trackTintColor=Metin2ColorHex(0x080B11); self.progressView.progress=0.0f; self.progressView.translatesAutoresizingMaskIntoConstraints=NO; [card addSubview:self.progressView];
 self.detailLabel=[[UILabel alloc] init]; self.detailLabel.text=@""; self.detailLabel.textColor=Metin2ColorHex(0xB0BEC5); self.detailLabel.font=[UIFont systemFontOfSize:12.0f]; self.detailLabel.textAlignment=NSTextAlignmentCenter; self.detailLabel.translatesAutoresizingMaskIntoConstraints=NO; [card addSubview:self.detailLabel];
 self.retryButton=[UIButton buttonWithType:UIButtonTypeSystem]; [self.retryButton setTitle:@"RETRY" forState:UIControlStateNormal]; [self.retryButton setTitleColor:Metin2ColorHex(0x0B0F17) forState:UIControlStateNormal]; self.retryButton.titleLabel.font=[UIFont boldSystemFontOfSize:14.0f]; self.retryButton.backgroundColor=Metin2ColorHex(0xDAA520); self.retryButton.layer.cornerRadius=4.0f; self.retryButton.hidden=YES; self.retryButton.translatesAutoresizingMaskIntoConstraints=NO; [self.retryButton addTarget:self action:@selector(onRetryTap) forControlEvents:UIControlEventTouchUpInside]; [card addSubview:self.retryButton];
 [NSLayoutConstraint activateConstraints:@[[card.centerXAnchor constraintEqualToAnchor:self.view.centerXAnchor],[card.centerYAnchor constraintEqualToAnchor:self.view.centerYAnchor],[card.widthAnchor constraintEqualToConstant:440.0f],[title.topAnchor constraintEqualToAnchor:card.topAnchor constant:24.0f],[title.leadingAnchor constraintEqualToAnchor:card.leadingAnchor constant:20.0f],[title.trailingAnchor constraintEqualToAnchor:card.trailingAnchor constant:-20.0f],[self.statusLabel.topAnchor constraintEqualToAnchor:title.bottomAnchor constant:16.0f],[self.statusLabel.leadingAnchor constraintEqualToAnchor:card.leadingAnchor constant:20.0f],[self.statusLabel.trailingAnchor constraintEqualToAnchor:card.trailingAnchor constant:-20.0f],[self.progressView.topAnchor constraintEqualToAnchor:self.statusLabel.bottomAnchor constant:14.0f],[self.progressView.leadingAnchor constraintEqualToAnchor:card.leadingAnchor constant:24.0f],[self.progressView.trailingAnchor constraintEqualToAnchor:card.trailingAnchor constant:-24.0f],[self.progressView.heightAnchor constraintEqualToConstant:10.0f],[self.detailLabel.topAnchor constraintEqualToAnchor:self.progressView.bottomAnchor constant:10.0f],[self.detailLabel.leadingAnchor constraintEqualToAnchor:card.leadingAnchor constant:20.0f],[self.detailLabel.trailingAnchor constraintEqualToAnchor:card.trailingAnchor constant:-20.0f],[self.retryButton.topAnchor constraintEqualToAnchor:self.detailLabel.bottomAnchor constant:14.0f],[self.retryButton.centerXAnchor constraintEqualToAnchor:card.centerXAnchor],[self.retryButton.widthAnchor constraintEqualToConstant:140.0f],[self.retryButton.heightAnchor constraintEqualToConstant:38.0f],[self.retryButton.bottomAnchor constraintEqualToAnchor:card.bottomAnchor constant:-24.0f]]];
 [self startPatchCheck];
}
- (void)onRetryTap { self.retryButton.hidden=YES; [self startPatchCheck]; }
- (void)updateUIStatus:(NSString*)s detail:(NSString*)d progress:(float)p showRetry:(BOOL)r { dispatch_async(dispatch_get_main_queue(),^{ self.statusLabel.text=s; self.detailLabel.text=d; if(p>=0.0f) self.progressView.progress=p; self.retryButton.hidden=!r; }); }
- (void)startPatchCheck { if(self.isWorkerRunning) return; self.isWorkerRunning=YES; [self updateUIStatus:@"Connecting to patch server..." detail:@"" progress:0.0f showRetry:NO]; dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0),^{ [self runPatchWorker]; self.isWorkerRunning=NO; }); }
- (NSString*)computeMd5:(NSString*)path { NSInputStream* st=[NSInputStream inputStreamWithFileAtPath:path]; if(!st) return @""; [st open]; CC_MD5_CTX c; CC_MD5_Init(&c); uint8_t b[131072]; while([st hasBytesAvailable]){ NSInteger n=[st read:b maxLength:sizeof(b)]; if(n<=0) break; CC_MD5_Update(&c,b,(CC_LONG)n); } [st close]; unsigned char dg[CC_MD5_DIGEST_LENGTH]; CC_MD5_Final(dg,&c); NSMutableString* h=[NSMutableString stringWithCapacity:32]; for(int i=0;i<CC_MD5_DIGEST_LENGTH;++i) [h appendFormat:@"%02x",dg[i]]; return h; }
- (NSData*)fetchManifest:(NSString*)u { for(int a=1;a<=4;++a){ if(a>1) [self updateUIStatus:[NSString stringWithFormat:@"Connecting... (%d/4)",a] detail:@"Retrying..." progress:0.0f showRetry:NO]; dispatch_semaphore_t sem=dispatch_semaphore_create(0); __block NSData* outData=nil; NSMutableURLRequest* req=[NSMutableURLRequest requestWithURL:[NSURL URLWithString:u] cachePolicy:NSURLRequestReloadIgnoringLocalCacheData timeoutInterval:10.0]; [req setValue:@"no-cache" forHTTPHeaderField:@"Cache-Control"]; [[[NSURLSession sharedSession] dataTaskWithRequest:req completionHandler:^(NSData* d,NSURLResponse* r,NSError* e){ if(!e&&[r isKindOfClass:[NSHTTPURLResponse class]]&&((NSHTTPURLResponse*)r).statusCode==200&&d&&d.length>0) outData=[[NSData alloc] initWithData:d]; dispatch_semaphore_signal(sem); }] resume]; dispatch_semaphore_wait(sem,DISPATCH_TIME_FOREVER); if(outData) return outData; [NSThread sleepForTimeInterval:0.6]; } return nil; }
- (BOOL)downloadEntry:(NSDictionary*)entry toPath:(NSString*)target base:(long long)base total:(long long)total idx:(int)idx count:(int)count err:(NSString**)err { NSString* rel=entry[@"path"]; NSString* expMd5=[entry[@"md5"] lowercaseString]; NSString* tmp=[target stringByAppendingString:@".tmp"]; NSFileManager* fm=[NSFileManager defaultManager]; NSTimeInterval t0=[NSDate timeIntervalSinceReferenceDate]; for(int a=1;a<=4;++a){ [fm removeItemAtPath:tmp error:nil]; NSURLSessionConfiguration* cfg=[NSURLSessionConfiguration ephemeralSessionConfiguration]; cfg.timeoutIntervalForRequest=25.0; cfg.timeoutIntervalForResource=7200.0; NSURLSession* sess=[NSURLSession sessionWithConfiguration:cfg]; NSMutableURLRequest* req=[NSMutableURLRequest requestWithURL:[NSURL URLWithString:[kPatchBaseUrl stringByAppendingString:[rel stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet URLPathAllowedCharacterSet]]]] cachePolicy:NSURLRequestReloadIgnoringLocalCacheData timeoutInterval:25.0]; dispatch_semaphore_t sem=dispatch_semaphore_create(0); __block BOOL ok=NO; NSURLSessionDownloadTask* task=[sess downloadTaskWithRequest:req completionHandler:^(NSURL* loc,NSURLResponse* r,NSError* e){ if(!e&&loc&&((NSHTTPURLResponse*)r).statusCode==200){ [fm removeItemAtPath:tmp error:nil]; ok=[fm moveItemAtURL:loc toURL:[NSURL fileURLWithPath:tmp] error:nil]; } dispatch_semaphore_signal(sem); }]; [task resume]; while(dispatch_semaphore_wait(sem,dispatch_time(DISPATCH_TIME_NOW,80000000LL))!=0){ long long cur=base+task.countOfBytesReceived; double dt=MAX(0.1,[NSDate timeIntervalSinceReferenceDate]-t0); double spd=(cur/1048576.0)/dt; float prg=total>0?(float)((double)cur/(double)total):1.0f; [self updateUIStatus:[NSString stringWithFormat:@"Downloading: %@ (%d/%d)",rel,idx,count] detail:[NSString stringWithFormat:@"%.1f / %.1f MB (%.2f MB/s)",cur/1048576.0,total/1048576.0,spd] progress:prg showRetry:NO]; } [sess finishTasksAndInvalidate]; if(!ok){ if(a<4){ [self updateUIStatus:[NSString stringWithFormat:@"Reconnecting (%d/4): %@",a,rel] detail:@"Retrying..." progress:-1.0f showRetry:NO]; [NSThread sleepForTimeInterval:0.8]; } continue; } if(![[[self computeMd5:tmp] lowercaseString] isEqualToString:expMd5]){ [fm removeItemAtPath:tmp error:nil]; if(a==4&&err) *err=[NSString stringWithFormat:@"MD5 mismatch: %@",rel]; continue; } [fm removeItemAtPath:target error:nil]; if(![fm moveItemAtPath:tmp toPath:target error:nil]){ if(err) *err=[NSString stringWithFormat:@"Save error: %@",rel]; return NO; } return YES; } if(err&&!*err) *err=[NSString stringWithFormat:@"Download failed: %@",rel]; return NO; }
- (void)runPatchWorker { NSString* base=[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES) firstObject]; NSFileManager* fm=[NSFileManager defaultManager]; for(NSString* d in @[@"UserData",@"UserData/environment",@"shop",@"temp",@"upload",@"mark"]) [fm createDirectoryAtPath:[base stringByAppendingPathComponent:d] withIntermediateDirectories:YES attributes:nil error:nil]; NSData* mData=[self fetchManifest:[NSString stringWithFormat:@"%@manifest.json?t=%lld",kPatchBaseUrl,(long long)([[NSDate date] timeIntervalSince1970]*1000)]]; if(!mData){ [self updateUIStatus:@"Update Error: Connection timed out" detail:@"Tap RETRY to continue." progress:0.0f showRetry:YES]; return; } NSDictionary* root=[NSJSONSerialization JSONObjectWithData:mData options:0 error:nil]; NSArray* files=root[@"files"]; if(![files isKindOfClass:[NSArray class]]){ [self updateUIStatus:@"Update Error: Invalid manifest" detail:@"Tap RETRY to continue." progress:0.0f showRetry:YES]; return; } NSUserDefaults* def=[NSUserDefaults standardUserDefaults]; NSMutableDictionary* cache=[[def dictionaryForKey:kPatchCachePrefKey] mutableCopy]?:[NSMutableDictionary dictionary]; NSMutableArray* todo=[NSMutableArray array]; long long totalBytes=0; int cnt=(int)files.count; for(int i=0;i<cnt;++i){ NSDictionary* it=files[i]; NSString* rel=it[@"path"]; long long sz=[it[@"size"] longLongValue]; NSString* md5=[it[@"md5"] lowercaseString]; if(i%8==0||i==cnt-1) [self updateUIStatus:[NSString stringWithFormat:@"Verifying files (%d/%d)...",i+1,cnt] detail:rel progress:(float)(i+1)/(float)cnt showRetry:NO]; NSString* loc=[base stringByAppendingPathComponent:rel]; BOOL ok=NO; if([fm fileExistsAtPath:loc]){ NSDictionary* at=[fm attributesOfItemAtPath:loc error:nil]; if([at fileSize]==sz){ long long mod=(long long)[[at fileModificationDate] timeIntervalSince1970]; NSString* cVal=[NSString stringWithFormat:@"%lld:%lld:%@",sz,mod,md5]; if([cache[rel] isEqualToString:cVal]) ok=YES; else if([[[self computeMd5:loc] lowercaseString] isEqualToString:md5]){ cache[rel]=cVal; ok=YES; } } } if(!ok){ [todo addObject:it]; totalBytes+=sz; } } [def setObject:cache forKey:kPatchCachePrefKey]; [def synchronize]; long long doneBytes=0; for(int i=0;i<(int)todo.count;++i){ NSDictionary* it=todo[i]; NSString* rel=it[@"path"]; NSString* dst=[base stringByAppendingPathComponent:rel]; [fm createDirectoryAtPath:[dst stringByDeletingLastPathComponent] withIntermediateDirectories:YES attributes:nil error:nil]; NSString* err=nil; if(![self downloadEntry:it toPath:dst base:doneBytes total:totalBytes idx:i+1 count:(int)todo.count err:&err]){ [self updateUIStatus:[NSString stringWithFormat:@"Update Error: %@",err?:@"Failed"] detail:@"Tap RETRY to continue." progress:0.0f showRetry:YES]; return; } doneBytes+=[it[@"size"] longLongValue]; NSDictionary* at=[fm attributesOfItemAtPath:dst error:nil]; cache[rel]=[NSString stringWithFormat:@"%lld:%lld:%@",[it[@"size"] longLongValue],(long long)[[at fileModificationDate] timeIntervalSince1970],[it[@"md5"] lowercaseString]]; [def setObject:cache forKey:kPatchCachePrefKey]; } [def synchronize]; [self updateUIStatus:@"Client is up to date! Launching..." detail:@"" progress:1.0f showRetry:NO]; dispatch_after(dispatch_time(DISPATCH_TIME_NOW,250000000LL),dispatch_get_main_queue(),^{ chdir([base UTF8String]); setenv("METIN2_IOS_DOCS_DIR",[base UTF8String],1); UIWindow* w=[UIApplication sharedApplication].delegate.window; if(!w) w=[UIApplication sharedApplication].windows.firstObject; w.rootViewController=[[Metin2IOSViewController alloc] init]; [w makeKeyAndVisible]; }); }
- (UIInterfaceOrientationMask)supportedInterfaceOrientations { return UIInterfaceOrientationMaskLandscape; }
- (BOOL)prefersStatusBarHidden { return YES; }
@end
@interface Metin2AppDelegate : UIResponder <UIApplicationDelegate>
@property (nonatomic,strong) UIWindow* window;
@end
@implementation Metin2AppDelegate
- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions { NSString* docs=[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES) firstObject]; chdir([docs UTF8String]); setenv("METIN2_IOS_DOCS_DIR",[docs UTF8String],1); self.window=[[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds]; self.window.rootViewController=[[Metin2PatcherViewController alloc] init]; [self.window makeKeyAndVisible]; return YES; }
- (UIInterfaceOrientationMask)application:(UIApplication*)application supportedInterfaceOrientationsForWindow:(UIWindow*)window { return UIInterfaceOrientationMaskLandscape; }
@end
int main(int argc,char* argv[]){ @autoreleasepool { return UIApplicationMain(argc,argv,nil,NSStringFromClass([Metin2AppDelegate class])); } }
#endif
