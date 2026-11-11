#include "StdAfx.h"
#ifdef __ANDROID__
#include "PythonApplication.h"
#include "PythonCharacterManager.h"
#include "PythonPlayer.h"
#include "PythonExceptionSender.h"
#pragma pack()
#include "../EterBase/lzo.h"
#include "../EterPack/EterPackManager.h"
#include "../EterLib/StateManager.h"
#include "../EterLib/ResourceManager.h"
#include "../EterPythonLib/PythonWindow.h"
#if __has_include("../EterLib/CRenderTargetManager.h")
#include "../EterLib/CRenderTargetManager.h"
#define ANDROID_HAS_RENDERTARGET_MGR 1
#elif __has_include("CRenderTargetManager.h")
#include "CRenderTargetManager.h"
#define ANDROID_HAS_RENDERTARGET_MGR 1
#elif __has_include("../EterLib/RenderTargetManager.h")
#include "../EterLib/RenderTargetManager.h"
#define ANDROID_HAS_RENDERTARGET_MGR 1
#endif
#include "Locale.h"
#ifdef ENABLE_VOICE_CHAT
#include "VoiceChat.h"
#endif
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#if !defined(__APPLE__)
#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#define LOG_TAG "Metin2Mobile"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
#include <stdio.h>
typedef void JNIEnv;
typedef void* jobject;
typedef int jint;
typedef float jfloat;
typedef unsigned char jboolean;
typedef void JavaVM;
typedef void AAssetManager;
#define JNIEXPORT extern "C"
#define JNICALL
#define LOGI(...) do { fprintf(stdout, "[Metin2iOS] "); fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); } while(0)
#define LOGE(...) do { fprintf(stderr, "[Metin2iOS ERR] "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while(0)

extern "C" {
    bool IOSDecodeImageFromMemory(const void* data, unsigned int size, unsigned int* outW, unsigned int* outH, std::vector<uint32_t>& outBGRA);
    bool IOSRenderCharGlyph(int pxSize, unsigned short ch, int* outW, int* outH, int* outAdv, uint32_t* outPixels64);
    void IOSToggleSoftKeyboard(bool show);
    void IOSSetMusicVolume(float volume);
    void IOSStopMusic();
    void IOSPlayAudioFile(const char* diskPath, bool isLoopMusic, float volume);
    const char* IOSGetDocumentsDirectory();
}
#endif
#include <IL/il.h>
#include <SpeedTreeRT.h>
#include <cstdarg>
#include <dlfcn.h>
#include <signal.h>
#ifdef __APPLE__
#include <sys/ucontext.h>
#else
#include <ucontext.h>
#endif
#include <unwind.h>
#include <Python27/frameobject.h>

// Forward declarations from UserInterface.cpp
extern bool PackInitialize(const char * c_pszFolder);
extern bool RunMainScript(CPythonLauncher& pyLauncher, const char* lpCmdLine);
extern bool SetDefaultCodePage(DWORD codePage);
extern void ApplicationStringTable_Initialize(HINSTANCE hInstance);

// Global Asset Manager pointer
AAssetManager* g_pAssetManager = nullptr;

// Random number generator stubs matching eterbase/Random.h
unsigned long metin2_random() {
    return (unsigned long)rand();
}

void metin2_srandom(unsigned long seed) {
    srand((unsigned int)seed);
}

// Out-of-line base constructor/destructor & vtable for amun::VoiceChat
#ifdef ENABLE_VOICE_CHAT
namespace amun {
    VoiceChat::VoiceChat() {}
    VoiceChat::~VoiceChat() {}
}
#endif

extern "C" {

// Missing Windows Text Services Framework (TSF) GUIDs
extern const GUID CLSID_TF_InputProcessorProfiles = { 0x1F02B6C5, 0x7842, 0x4EE6, { 0x8A, 0x0B, 0x9A, 0x24, 0x18, 0x3A, 0x95, 0xCA } };
extern const GUID GUID_TFCAT_TIP_KEYBOARD = { 0x34745C63, 0xB2F0, 0x4784, { 0x8B, 0x67, 0x5E, 0x12, 0xC8, 0x70, 0x1A, 0x31 } };
extern const GUID GUID_COMPARTMENT_KEYBOARD_OPENCLOSE = { 0x58273AAD, 0x01BB, 0x4164, { 0x95, 0xC6, 0x75, 0x5B, 0xA0, 0xB5, 0x16, 0x2D } };

static bool s_bEngineReady = false;
int g_AndroidPhysWidth = 1280;
int g_AndroidPhysHeight = 720;
int g_AndroidScreenWidth = 1280;
int g_AndroidScreenHeight = 720;
int g_AndroidMouseX = 640;
int g_AndroidMouseY = 360;
static JavaVM* g_pJavaVM = nullptr;

bool AndroidDecodeImageFromMemory(const void* data, unsigned int size, unsigned int* outW, unsigned int* outH, std::vector<uint32_t>& outBGRA) {
#ifdef __APPLE__
    return IOSDecodeImageFromMemory(data, size, outW, outH, outBGRA);
#else
    if (!g_pJavaVM || !data || size < 8) return false;
    JNIEnv* env = nullptr;
    if (g_pJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK || !env) return false;

    static jclass s_bmpFactoryCls = nullptr;
    static jmethodID s_decodeMethod = nullptr;
    static jclass s_bmpCls = nullptr;
    static jmethodID s_getWidthMid = nullptr;
    static jmethodID s_getHeightMid = nullptr;
    static jmethodID s_getPixelsMid = nullptr;
    static jmethodID s_recycleMid = nullptr;

    if (!s_bmpFactoryCls) {
        jclass localFactory = env->FindClass("android/graphics/BitmapFactory");
        if (!localFactory) { env->ExceptionClear(); return false; }
        s_bmpFactoryCls = (jclass)env->NewGlobalRef(localFactory);
        env->DeleteLocalRef(localFactory);
        s_decodeMethod = env->GetStaticMethodID(s_bmpFactoryCls, "decodeByteArray", "([BII)Landroid/graphics/Bitmap;");

        jclass localBmp = env->FindClass("android/graphics/Bitmap");
        if (localBmp) {
            s_bmpCls = (jclass)env->NewGlobalRef(localBmp);
            env->DeleteLocalRef(localBmp);
            s_getWidthMid = env->GetMethodID(s_bmpCls, "getWidth", "()I");
            s_getHeightMid = env->GetMethodID(s_bmpCls, "getHeight", "()I");
            s_getPixelsMid = env->GetMethodID(s_bmpCls, "getPixels", "([IIIIIII)V");
            s_recycleMid = env->GetMethodID(s_bmpCls, "recycle", "()V");
        }
    }
    if (!s_decodeMethod || !s_bmpCls) return false;

    jbyteArray byteArray = env->NewByteArray((jsize)size);
    if (!byteArray) { env->ExceptionClear(); return false; }
    env->SetByteArrayRegion(byteArray, 0, (jsize)size, (const jbyte*)data);

    jobject bitmap = env->CallStaticObjectMethod(s_bmpFactoryCls, s_decodeMethod, byteArray, 0, (jint)size);
    env->DeleteLocalRef(byteArray);
    if (env->ExceptionCheck() || !bitmap) { env->ExceptionClear(); return false; }

    jint w = env->CallIntMethod(bitmap, s_getWidthMid);
    jint h = env->CallIntMethod(bitmap, s_getHeightMid);
    bool ok = false;

    if (w > 0 && h > 0 && w <= 4096 && h <= 4096) {
        jintArray pixels = env->NewIntArray(w * h);
        if (pixels && !env->ExceptionCheck()) {
            env->CallVoidMethod(bitmap, s_getPixelsMid, pixels, 0, w, 0, 0, w, h);
            if (!env->ExceptionCheck()) {
                *outW = (unsigned int)w;
                *outH = (unsigned int)h;
                outBGRA.resize((size_t)w * (size_t)h);
                env->GetIntArrayRegion(pixels, 0, w * h, (jint*)outBGRA.data());
                ok = true;
            }
            env->DeleteLocalRef(pixels);
        }
    }
    env->CallVoidMethod(bitmap, s_recycleMid);
    env->DeleteLocalRef(bitmap);
    env->ExceptionClear();
    return ok;
#endif
}

bool AndroidRenderCharGlyph(int fontSize, unsigned short ch, int* outW, int* outH, int* outAdv, uint32_t* outPixels64) {
#ifndef __APPLE__
    if (!g_pJavaVM) return false;
#endif
    if (!outW || !outH || !outAdv || !outPixels64) return false;

    int pxSize = (fontSize < 0) ? -fontSize : (fontSize > 0 ? fontSize : 14);
    if (pxSize < 11) pxSize = 12;
    if (pxSize > 40) pxSize = 40;

    struct SGlyphCacheEntry {
        int w, h, adv;
        std::vector<uint32_t> rows; // 64 * h pixels
    };
    static std::unordered_map<uint32_t, SGlyphCacheEntry> s_glyphCache;
    uint32_t key = ((uint32_t)pxSize << 16) | (uint32_t)ch;
    auto it = s_glyphCache.find(key);
    if (it != s_glyphCache.end()) {
        const SGlyphCacheEntry& e = it->second;
        *outW = e.w;
        *outH = e.h;
        *outAdv = e.adv;
        memcpy(outPixels64, e.rows.data(), e.rows.size() * sizeof(uint32_t));
        return true;
    }

#ifdef __APPLE__
    if (!IOSRenderCharGlyph(pxSize, ch, outW, outH, outAdv, outPixels64))
        return false;
    int w = *outW;
    int h = *outH;
#else
    JNIEnv* env = nullptr;
    if (g_pJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK || !env) return false;

    static jobject s_paint = nullptr, s_bmp = nullptr, s_canvas = nullptr;
    static jintArray s_pixArr = nullptr;
    static jmethodID s_setTextSize = nullptr, s_measureText = nullptr, s_ascent = nullptr, s_descent = nullptr;
    static jmethodID s_drawText = nullptr, s_eraseColor = nullptr, s_getPixels = nullptr;
    static int s_lastPxSize = -1;

    if (!s_paint) {
        jclass paintCls = env->FindClass("android/graphics/Paint");
        jmethodID paintCtor = env->GetMethodID(paintCls, "<init>", "(I)V");
        jobject p = env->NewObject(paintCls, paintCtor, 1 | 128); // ANTI_ALIAS_FLAG | SUBPIXEL_TEXT_FLAG
        env->CallVoidMethod(p, env->GetMethodID(paintCls, "setColor", "(I)V"), (jint)0xFFFFFFFF);
        s_paint = env->NewGlobalRef(p);
        s_setTextSize = env->GetMethodID(paintCls, "setTextSize", "(F)V");
        s_measureText = env->GetMethodID(paintCls, "measureText", "(Ljava/lang/String;)F");
        s_ascent = env->GetMethodID(paintCls, "ascent", "()F");
        s_descent = env->GetMethodID(paintCls, "descent", "()F");

        jclass bmpCls = env->FindClass("android/graphics/Bitmap");
        jclass cfgCls = env->FindClass("android/graphics/Bitmap$Config");
        jobject argb8888 = env->GetStaticObjectField(cfgCls, env->GetStaticFieldID(cfgCls, "ARGB_8888", "Landroid/graphics/Bitmap$Config;"));
        jobject b = env->CallStaticObjectMethod(bmpCls, env->GetStaticMethodID(bmpCls, "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;"), 64, 64, argb8888);
        s_bmp = env->NewGlobalRef(b);
        s_eraseColor = env->GetMethodID(bmpCls, "eraseColor", "(I)V");
        s_getPixels = env->GetMethodID(bmpCls, "getPixels", "([IIIIIII)V");

        jclass canvasCls = env->FindClass("android/graphics/Canvas");
        jobject c = env->NewObject(canvasCls, env->GetMethodID(canvasCls, "<init>", "(Landroid/graphics/Bitmap;)V"), b);
        s_canvas = env->NewGlobalRef(c);
        s_drawText = env->GetMethodID(canvasCls, "drawText", "(Ljava/lang/String;FFLandroid/graphics/Paint;)V");

        jintArray pa = env->NewIntArray(64 * 64);
        s_pixArr = (jintArray)env->NewGlobalRef(pa);

        env->DeleteLocalRef(pa); env->DeleteLocalRef(c); env->DeleteLocalRef(canvasCls);
        env->DeleteLocalRef(b); env->DeleteLocalRef(argb8888); env->DeleteLocalRef(cfgCls);
        env->DeleteLocalRef(bmpCls); env->DeleteLocalRef(p); env->DeleteLocalRef(paintCls);
    }

    if (s_lastPxSize != pxSize) {
        s_lastPxSize = pxSize;
        env->CallVoidMethod(s_paint, s_setTextSize, (jfloat)pxSize);
    }
    jchar jc = (jchar)ch;
    jstring jstr = env->NewString(&jc, 1);
    jfloat adv = env->CallFloatMethod(s_paint, s_measureText, jstr);
    jfloat ascent = env->CallFloatMethod(s_paint, s_ascent);
    jfloat descent = env->CallFloatMethod(s_paint, s_descent);

    int w = (int)ceilf(adv) + 2;
    int h = (int)ceilf(descent - ascent) + 2;
    if (w < 4) w = 4; else if (w > 64) w = 64;
    if (h < 8) h = 8; else if (h > 64) h = 64;

    *outW = w;
    *outH = h;
    *outAdv = (int)ceilf(adv);

    env->CallVoidMethod(s_bmp, s_eraseColor, (jint)0);
    env->CallVoidMethod(s_canvas, s_drawText, jstr, 0.5f, -ascent + 0.5f, s_paint);
    env->CallVoidMethod(s_bmp, s_getPixels, s_pixArr, 0, 64, 0, 0, 64, h);
    env->GetIntArrayRegion(s_pixArr, 0, 64 * h, (jint*)outPixels64);

    env->DeleteLocalRef(jstr);
    env->ExceptionClear();
#endif

    if (s_glyphCache.size() > 2048) s_glyphCache.clear();
    SGlyphCacheEntry entry;
    entry.w = w;
    entry.h = h;
    entry.adv = *outAdv;
    entry.rows.assign(outPixels64, outPixels64 + (size_t)(64 * h));
    s_glyphCache.emplace(key, std::move(entry));
    return true;
}

#ifdef __APPLE__
#include <mach/mach.h>
#include <cxxabi.h>
#include <fcntl.h>
#include <exception>
#endif

static volatile const char* g_szCrashStage = "Startup";
static char g_szLastCCall[256] = "None";
static char g_szBreadcrumbs[20][160] = {{0}};
static volatile uint32_t g_uBreadcrumbIdx = 0;

static void RecordBreadcrumb(const char* msg) {
    if (!msg) return;
    uint32_t slot = (__atomic_fetch_add(&g_uBreadcrumbIdx, 1, __ATOMIC_RELAXED)) % 20;
    snprintf(g_szBreadcrumbs[slot], sizeof(g_szBreadcrumbs[slot]), "%s", msg);
}

static int MobilePyProfileTracer(PyObject* /*obj*/, PyFrameObject* frame, int what, PyObject* arg) {
    if (what == PyTrace_C_CALL && arg && PyCFunction_Check(arg)) {
        const char* cfunc = ((PyCFunctionObject*)arg)->m_ml ? ((PyCFunctionObject*)arg)->m_ml->ml_name : "?";
        const char* fname = (frame && frame->f_code && frame->f_code->co_filename) ? PyString_AsString(frame->f_code->co_filename) : "?";
        const char* fn = (frame && frame->f_code && frame->f_code->co_name) ? PyString_AsString(frame->f_code->co_name) : "?";
        int line = frame ? PyFrame_GetLineNumber(frame) : 0;
        snprintf(g_szLastCCall, sizeof(g_szLastCCall), "C_CALL %s() at %s:%d (%s)", cfunc ? cfunc : "?", fname ? fname : "?", line, fn ? fn : "?");
        RecordBreadcrumb(g_szLastCCall);
    } else if (what == PyTrace_CALL && frame && frame->f_code) {
        const char* fname = frame->f_code->co_filename ? PyString_AsString(frame->f_code->co_filename) : "?";
        const char* fn = frame->f_code->co_name ? PyString_AsString(frame->f_code->co_name) : "?";
        int line = PyFrame_GetLineNumber(frame);
        char buf[160];
        snprintf(buf, sizeof(buf), "PY_CALL %s:%d in %s()", fname ? fname : "?", line, fn ? fn : "?");
        RecordBreadcrumb(buf);
    }
    return 0;
}

struct AndroidBacktraceState {
    void** current;
    void** end;
};

static _Unwind_Reason_Code AndroidUnwindCallback(struct _Unwind_Context* context, void* arg) {
    AndroidBacktraceState* state = static_cast<AndroidBacktraceState*>(arg);
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) {
        if (state->current == state->end) return _URC_END_OF_STACK;
        *state->current++ = reinterpret_cast<void*>(pc);
    }
    return _URC_NO_REASON;
}

static void FormatAddressSymbol(void* addr, char* outBuf, size_t outSize) {
    if (!addr) { snprintf(outBuf, outSize, "0x0 (NULL)"); return; }
    Dl_info dl{};
    if (dladdr(addr, &dl) && dl.dli_sname) {
        const char* sym = dl.dli_sname;
#ifdef __APPLE__
        int status = 0;
        char* demangled = abi::__cxa_demangle(dl.dli_sname, nullptr, nullptr, &status);
        if (status == 0 && demangled) sym = demangled;
#endif
        snprintf(outBuf, outSize, "%p (%s + 0x%zx)", addr, sym, (size_t)((uintptr_t)addr - (uintptr_t)dl.dli_saddr));
#ifdef __APPLE__
        if (demangled) free(demangled);
#endif
    } else if (dl.dli_fname) {
        snprintf(outBuf, outSize, "%p (%s + 0x%zx)", addr, dl.dli_fname, (size_t)((uintptr_t)addr - (uintptr_t)dl.dli_fbase));
    } else {
        snprintf(outBuf, outSize, "%p (unknown)", addr);
    }
}

static void WriteCrashLine(int fd, const char* fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf) - 2, fmt, ap);
    va_end(ap);
    if (n > 0) {
        if (n > (int)sizeof(buf) - 2) n = (int)sizeof(buf) - 2;
        buf[n++] = '\n';
        buf[n] = '\0';
        if (fd >= 0) write(fd, buf, (size_t)n);
        fprintf(stderr, "%s", buf);
    }
}

static const char* SignalName(int sig, int code) {
    switch (sig) {
        case SIGSEGV: return (code == SEGV_MAPERR) ? "SIGSEGV (SEGV_MAPERR: Unmapped/NULL memory read/write)" : "SIGSEGV (SEGV_ACCERR: Invalid memory permissions/alignment)";
        case SIGBUS:  return (code == BUS_ADRALN)  ? "SIGBUS (BUS_ADRALN: Unaligned memory access)" : "SIGBUS (Invalid physical/mapped memory address)";
        case SIGABRT: return "SIGABRT (Abort / Assert / Uncaught Exception / Heap Corruption)";
        case SIGILL:  return "SIGILL (Illegal Instruction)";
        case SIGTRAP: return "SIGTRAP (Breakpoint / Trap)";
        case SIGFPE:  return "SIGFPE (Floating Point / Divide by Zero)";
        default:      return "UNKNOWN SIGNAL";
    }
}

static void AndroidCrashHandler(int sig, siginfo_t* info, void* ucontext) {
    static volatile sig_atomic_t s_inCrashHandler = 0;
    if (s_inCrashHandler) _exit(1);
    s_inCrashHandler = 1;

    fflush(NULL);
    int fd = open("crash_report.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);

    WriteCrashLine(fd, "====================================================");
    WriteCrashLine(fd, "=== METIN2 iOS NATIVE CRASH & MEMORY DEBUG REPORT ===");
    WriteCrashLine(fd, "====================================================");
    WriteCrashLine(fd, "Signal      : %d -> %s", sig, SignalName(sig, info ? info->si_code : 0));
    WriteCrashLine(fd, "Fault Addr  : %p (si_code=%d, errno=%d)", info ? info->si_addr : nullptr, info ? info->si_code : 0, info ? info->si_errno : 0);
    WriteCrashLine(fd, "Engine Stage: %s", g_szCrashStage ? (const char*)g_szCrashStage : "?");
    WriteCrashLine(fd, "Last C_CALL : %s", g_szLastCCall);

#ifdef __APPLE__
    mach_task_basic_info_data_t taskInfo{};
    mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&taskInfo, &infoCount) == KERN_SUCCESS) {
        WriteCrashLine(fd, "RAM Usage   : Resident=%.1f MB | Peak=%.1f MB",
            (double)taskInfo.resident_size / (1024.0 * 1024.0),
            (double)taskInfo.resident_size_max / (1024.0 * 1024.0));
    }
#if defined(__aarch64__)
    ucontext_t* uc = (ucontext_t*)ucontext;
    if (uc && uc->uc_mcontext) {
        char pcSym[256], lrSym[256];
        FormatAddressSymbol((void*)uc->uc_mcontext->__ss.__pc, pcSym, sizeof(pcSym));
        FormatAddressSymbol((void*)uc->uc_mcontext->__ss.__lr, lrSym, sizeof(lrSym));
        WriteCrashLine(fd, "--- ARM64 CPU REGISTERS ---");
        WriteCrashLine(fd, "PC (Fault)  : %s", pcSym);
        WriteCrashLine(fd, "LR (Caller) : %s", lrSym);
        WriteCrashLine(fd, "SP=%p  FP=%p  FAR=%p  ESR=0x%08x",
            (void*)uc->uc_mcontext->__ss.__sp, (void*)uc->uc_mcontext->__ss.__fp,
            (void*)uc->uc_mcontext->__es.__far, (unsigned int)uc->uc_mcontext->__es.__esr);
        WriteCrashLine(fd, "X0=%p X1=%p X2=%p X3=%p",
            (void*)uc->uc_mcontext->__ss.__x[0], (void*)uc->uc_mcontext->__ss.__x[1],
            (void*)uc->uc_mcontext->__ss.__x[2], (void*)uc->uc_mcontext->__ss.__x[3]);
        WriteCrashLine(fd, "X4=%p X5=%p X6=%p X7=%p",
            (void*)uc->uc_mcontext->__ss.__x[4], (void*)uc->uc_mcontext->__ss.__x[5],
            (void*)uc->uc_mcontext->__ss.__x[6], (void*)uc->uc_mcontext->__ss.__x[7]);
    }
#endif
#endif

    WriteCrashLine(fd, "--- NATIVE C++ CALL STACK ---");
    void* buffer[32];
    AndroidBacktraceState state = { buffer, buffer + 32 };
    _Unwind_Backtrace(AndroidUnwindCallback, &state);
    size_t count = state.current - buffer;
    for (size_t idx = 0; idx < count; ++idx) {
        char symBuf[256];
        FormatAddressSymbol(buffer[idx], symBuf, sizeof(symBuf));
        WriteCrashLine(fd, "#%02zu %s", idx, symBuf);
    }

    WriteCrashLine(fd, "--- LAST 20 PYTHON / C++ BREADCRUMBS ---");
    uint32_t curIdx = g_uBreadcrumbIdx;
    uint32_t total = (curIdx < 20) ? curIdx : 20;
    for (uint32_t i = 0; i < total; ++i) {
        uint32_t slot = (curIdx - 1 - i) % 20;
        if (g_szBreadcrumbs[slot][0])
            WriteCrashLine(fd, "[-%02u] %s", i, g_szBreadcrumbs[slot]);
    }

    WriteCrashLine(fd, "--- ACTIVE PYTHON FRAME STACK ---");
    PyFrameObject* f = PyEval_GetFrame();
    int depth = 0;
    while (f && depth < 20) {
        const char* fname = (f->f_code && f->f_code->co_filename) ? PyString_AsString(f->f_code->co_filename) : "?";
        const char* func = (f->f_code && f->f_code->co_name) ? PyString_AsString(f->f_code->co_name) : "?";
        int line = PyFrame_GetLineNumber(f);
        WriteCrashLine(fd, "PyFrame #%02d: %s:%d in %s()", depth, fname ? fname : "?", line, func ? func : "?");
        f = f->f_back;
        ++depth;
    }

    auto AppendTailFile = [&](const char* title, const char* path) {
        WriteCrashLine(fd, "--- %s ---", title);
        int lfd = open(path, O_RDONLY);
        if (lfd >= 0) {
            off_t sz = lseek(lfd, 0, SEEK_END);
            off_t start = (sz > 3500) ? (sz - 3500) : 0;
            lseek(lfd, start, SEEK_SET);
            char tbuf[3501];
            ssize_t r = read(lfd, tbuf, 3500);
            if (r > 0 && fd >= 0) {
                write(fd, tbuf, (size_t)r);
                write(fd, "\n", 1);
            }
            close(lfd);
        } else {
            WriteCrashLine(fd, "[Could not open %s]", path);
        }
    };
    AppendTailFile("TAIL OF SYSERR.TXT", "syserr.txt");
    AppendTailFile("TAIL OF LOG.TXT", "log.txt");

    if (fd >= 0) {
        fsync(fd);
        close(fd);
    }
    fflush(NULL);
    _exit(1);
}

static void InstallAndroidCrashHandler() {
    signal(SIGPIPE, SIG_IGN);
    struct sigaction sa{};
    sa.sa_sigaction = AndroidCrashHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGBUS, &sa, nullptr);
    sigaction(SIGILL, &sa, nullptr);
    sigaction(SIGTRAP, &sa, nullptr);
    sigaction(SIGFPE, &sa, nullptr);
    std::set_terminate([]() {
        g_szCrashStage = "std::terminate (Uncaught C++ Exception)";
        abort();
    });
}

#ifdef ENABLE_OPENGL
void initmobileInput(void);
#endif

static const char* GetMobileBaseDir() {
#ifdef __APPLE__
    return IOSGetDocumentsDirectory();
#else
    return "/sdcard/Android/data/com.metin2.client/files";
#endif
}

JNIEXPORT void JNICALL Java_com_metin2_client_NativeLib_init(JNIEnv* env, jobject obj, jobject assetManager, jint width, jint height) {
    LOGI("Initializing Metin2 Mobile Engine (%dx%d)", width, height);
    srand((unsigned int)time(NULL) ^ (unsigned int)getpid());
	if (width > 0 && height > 0) {
        g_AndroidPhysWidth = (int)width;
        g_AndroidPhysHeight = (int)height;
        g_AndroidScreenHeight = 720;
        g_AndroidScreenWidth = (int)(((float)width * 720.0f) / (float)height + 0.5f);
        if (g_AndroidScreenWidth < 960) g_AndroidScreenWidth = 960;
    }
    s_bEngineReady = false;
    
    #if !defined(__APPLE__)
    g_pAssetManager = AAssetManager_fromJava(env, assetManager);
#else
    (void)env; (void)obj; (void)assetManager;
#endif

    const char* targetDir = GetMobileBaseDir();
    if (chdir(targetDir) != 0) {
        LOGE("chdir to %s failed! errno=%d", targetDir, errno);
    } else {
        LOGI("Working directory set to %s", targetDir);
    }

    mkdir("upload", 0777);
    mkdir("mark", 0777);
    mkdir("temp", 0777);
#ifdef ENABLE_OPENGL
    void GenerateMobileHUDTextures();
    GenerateMobileHUDTextures();
#endif
    mkdir("lib/python2.7", 0777);
    mkdir("lib/python2.7/config", 0777);
    mkdir("include", 0777);
    mkdir("include/python2.7", 0777);
#if !defined(__APPLE__)
    env->GetJavaVM(&g_pJavaVM);
#endif

    if (width > 0 && height > 0 && access("metin2.cfg", F_OK) != 0) {
        if (FILE* cfgFp = fopen("metin2.cfg", "wb")) {
            fprintf(cfgFp, "WIDTH\t\t\t\t\t\t%d\nHEIGHT\t\t\t\t\t\t%d\nBPP\t\t\t\t\t\t32\nFREQUENCY\t\t\t\t\t60\nSOFTWARE_CURSOR\t\t\t0\nSOFTWARE_TILING\t\t\t0\nOBJECT_CULLING\t\t\t\t1\nVISIBILITY\t\t\t\t\t3\nMUSIC_VOLUME\t\t\t\t0.000\nVOICE_VOLUME\t\t\t\t0\nGAMMA\t\t\t\t\t\t3\nIS_SAVE_ID\t\t\t\t\t0\nSAVE_ID\t\t\t\t\t0\nDECOMPRESSED_TEXTURE\t\t0\nWINDOWED\t\t\t\t\t1\n", (int)width, (int)height);
            fclose(cfgFp);
        }
    }
	if (access("audio_init_v1.flag", F_OK) != 0) {
        if (FILE* f = fopen("audio_init_v1.flag", "wb")) fclose(f);
        if (FILE* cfgFp = fopen("metin2.cfg", "rb")) {
            char buf[2048] = {0};
            size_t n = fread(buf, 1, sizeof(buf) - 1, cfgFp);
            fclose(cfgFp);
            std::string s(buf, n);
            size_t posM = s.find("MUSIC_VOLUME\t\t\t\t\t0.000");
            if (posM != std::string::npos) s.replace(posM, 23, "MUSIC_VOLUME\t\t\t\t\t0.500");
            size_t posV = s.find("VOICE_VOLUME\t\t\t\t\t0");
            if (posV != std::string::npos) s.replace(posV, 19, "VOICE_VOLUME\t\t\t\t\t3");
            if (FILE* outFp = fopen("metin2.cfg", "wb")) {
                fwrite(s.data(), 1, s.size(), outFp);
                fclose(outFp);
            }
        }
    }
    if (FILE* fp = fopen("lib/python2.7/config/Makefile", "a")) fclose(fp);
    if (FILE* fp = fopen("include/python2.7/pyconfig.h", "a")) fclose(fp);

    // 1. Open log.txt and syserr.txt
    SetLogLevel(1);
    InstallAndroidCrashHandler(); g_szCrashStage = "Init: OpenLogFile"; OpenLogFile(true);
    InstallAndroidCrashHandler();
    Tracen("Metin2 Mobile: Log initialized [BUILD v5 - IOS FIX].");

    // 2. Initialize string table and locale.cfg
    ApplicationStringTable_Initialize(NULL);
    LocaleService_LoadConfig("locale.cfg");
    SetDefaultCodePage(LocaleService_GetCodePage());

    // 3. Create required global singletons (CLZO and CEterPackManager)
    static CLZO* s_pLZO = nullptr;
    if (!CLZO::InstancePtr()) {
        s_pLZO = new CLZO();
    }

    static CEterPackManager* s_pEterPackManager = nullptr;
    if (!CEterPackManager::InstancePtr()) {
        s_pEterPackManager = new CEterPackManager();
    }

#ifdef ANDROID_HAS_RENDERTARGET_MGR
    static CRenderTargetManager* s_pRenderTargetManager = nullptr;
    if (!CRenderTargetManager::InstancePtr()) {
        s_pRenderTargetManager = new CRenderTargetManager();
    }
#endif

    // 4. Initialize pack/ folder
    g_szCrashStage = "Init: PackInitialize"; if (!PackInitialize("pack")) {
        LOGE("PackInitialize('pack') failed!");
        TraceError("PackInitialize('pack') failed!");
        return;
    }
    CEterPackManager::Instance().SetSearchMode(true);
    Tracen("Metin2 Mobile: PackInitialize('pack') succeeded.");

    if (LocaleService_LoadGlobal(NULL)) {
        SetDefaultCodePage(LocaleService_GetCodePage());
    }

    // 5. Initialize Python BEFORE constructing CPythonApplication (CWindowManager calls Py_BuildValue("()"))
    static CPythonLauncher s_pyLauncher;
    static CPythonExceptionSender s_pyExceptionSender;
    SetExceptionSender(&s_pyExceptionSender);

    static CPythonApplication* s_pApp = nullptr;
    if (!s_pApp) {
        s_pApp = new CPythonApplication();
    }
    g_szCrashStage = "Init: CPythonApplication::Initialize"; s_pApp->Initialize(NULL);

    Tracen("Metin2 Mobile: Starting CPythonLauncher...");
    if (s_pyLauncher.Create()) {
        PyRun_SimpleString(
            "import gc, sys, time as _time\n"
            "def _py_mktime(t):\n"
            "    y, m, d, H, M, S = int(t[0]), int(t[1]), int(t[2]), int(t[3]), int(t[4]), int(t[5])\n"
            "    if m <= 2: y -= 1; m += 12\n"
            "    days = 365*y + y//4 - y//100 + y//400 + (153*(m-3)+2)//5 + d - 719469\n"
            "    return float(days*86400 + H*3600 + M*60 + S)\n"
            "_time.mktime = getattr(_time, 'mktime', _py_mktime)\n"
            "_time.clock = getattr(_time, 'clock', _time.time)\n"
            "gc.disable()\n"
            "class _TD(object):\n"
            "    def __init__(self, days=0, seconds=0, microseconds=0, milliseconds=0, minutes=0, hours=0, weeks=0):\n"
            "        s = seconds + minutes*60 + hours*3600 + milliseconds/1000.0 + microseconds/1000000.0\n"
            "        self.days = int(days + weeks*7 + int(s // 86400))\n"
            "        self.seconds = int(s % 86400)\n"
            "        self.microseconds = int((s - int(s)) * 1000000)\n"
            "    def total_seconds(self): return self.days * 86400.0 + self.seconds + self.microseconds / 1000000.0\n"
            "class _DT(object):\n"
            "    def __init__(self, year=1970, month=1, day=1, hour=0, minute=0, second=0, microsecond=0):\n"
            "        self.year, self.month, self.day = int(year), int(month), int(day)\n"
            "        self.hour, self.minute, self.second, self.microsecond = int(hour), int(minute), int(second), int(microsecond)\n"
            "    def _ts(self): return _time.mktime((self.year, self.month, self.day, self.hour, self.minute, self.second, 0, 0, -1))\n"
            "    @classmethod\n"
            "    def fromtimestamp(cls, ts):\n"
            "        t = _time.localtime(ts)\n"
            "        return cls(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec)\n"
            "    @classmethod\n"
            "    def utcfromtimestamp(cls, ts):\n"
            "        t = _time.gmtime(ts)\n"
            "        return cls(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec)\n"
            "    @classmethod\n"
            "    def now(cls, tz=None): return cls.fromtimestamp(_time.time())\n"
            "    @classmethod\n"
            "    def today(cls): return cls.now()\n"
            "    @classmethod\n"
            "    def utcnow(cls): return cls.utcfromtimestamp(_time.time())\n"
            "    def timetuple(self): return _time.localtime(self._ts())\n"
            "    def weekday(self): return self.timetuple().tm_wday\n"
            "    def isoweekday(self): return self.timetuple().tm_wday + 1\n"
            "    def date(self): return _DT(self.year, self.month, self.day)\n"
            "    def strftime(self, fmt): return _time.strftime(fmt, self.timetuple())\n"
            "    def replace(self, **kw):\n"
            "        return _DT(kw.get('year', self.year), kw.get('month', self.month), kw.get('day', self.day),\n"
            "                   kw.get('hour', self.hour), kw.get('minute', self.minute), kw.get('second', self.second))\n"
            "    def __add__(self, o): return _DT.fromtimestamp(self._ts() + o.total_seconds()) if isinstance(o, _TD) else self\n"
            "    def __sub__(self, o):\n"
            "        if isinstance(o, _TD): return _DT.fromtimestamp(self._ts() - o.total_seconds())\n"
            "        return _TD(seconds=(self._ts() - o._ts()))\n"
            "    def __cmp__(self, o): return cmp(self._ts(), o._ts()) if isinstance(o, _DT) else -1\n"
            "    def __str__(self): return '%04d-%02d-%02d %02d:%02d:%02d' % (self.year, self.month, self.day, self.hour, self.minute, self.second)\n"
            "import imp\n"
            "_dt_mod = imp.new_module('datetime')\n"
            "_dt_mod.datetime = _DT\n"
            "_dt_mod.date = _DT\n"
            "_dt_mod.time = _DT\n"
            "_dt_mod.timedelta = _TD\n"
            "sys.modules['datetime'] = _dt_mod\n"
            "import _codecs\n"
            "def _mt2_codec(name):\n"
            "    return (_codecs.latin_1_encode, _codecs.latin_1_decode, None, None)\n"
            "_codecs.register(_mt2_codec)\n"
            "_log_mod = imp.new_module('logging')\n"
            "class _Logger(object):\n"
            "    def debug(self, *a, **k): pass\n"
            "    def info(self, *a, **k): pass\n"
            "    def warning(self, *a, **k): pass\n"
            "    def warn(self, *a, **k): pass\n"
            "    def error(self, *a, **k): pass\n"
            "    def critical(self, *a, **k): pass\n"
            "    def exception(self, *a, **k): pass\n"
            "    def log(self, *a, **k): pass\n"
            "    def addHandler(self, *a, **k): pass\n"
            "    def setLevel(self, *a, **k): pass\n"
            "_root_logger = _Logger()\n"
            "_log_mod.Logger = _Logger\n"
            "_log_mod.getLogger = lambda name=None: _root_logger\n"
            "_log_mod.basicConfig = lambda **k: None\n"
            "_log_mod.debug = _root_logger.debug\n"
            "_log_mod.info = _root_logger.info\n"
            "_log_mod.warning = _root_logger.warning\n"
            "_log_mod.warn = _root_logger.warn\n"
            "_log_mod.error = _root_logger.error\n"
            "_log_mod.critical = _root_logger.critical\n"
            "_log_mod.exception = _root_logger.exception\n"
            "_log_mod.log = _root_logger.log\n"
            "_log_mod.DEBUG = 10; _log_mod.INFO = 20; _log_mod.WARNING = 30; _log_mod.ERROR = 40; _log_mod.CRITICAL = 50\n"
            "sys.modules['logging'] = _log_mod\n"
        );
#ifdef ENABLE_OPENGL
        initmobileInput(); PyEval_SetProfile(MobilePyProfileTracer, NULL); g_szCrashStage = "Init: RunMainScript (system.py)";
#endif
        Tracen("Metin2 Mobile: Running RunMainScript (system.py)...");
        if (!RunMainScript(s_pyLauncher, "")) {
            if (PyErr_Occurred()) {
                PyErr_Print();
            }
            fflush(stderr);
            LOGE("RunMainScript failed! Check syserr.txt");
            TraceError("RunMainScript failed!");
            return;
        }
    } else {
        LOGE("s_pyLauncher.Create() failed!");
        TraceError("s_pyLauncher.Create() failed!");
        return;
    }
	PyRun_SimpleString(
        "import __builtin__, sys, imp, time as _t\n"
        "_t.clock = getattr(_t, 'clock', _t.time)\n"
        "_orig_init_builtin = imp.init_builtin\n"
        "def _fast_init_builtin(name):\n"
        "    m = sys.modules.get(name)\n"
        "    if m is None:\n"
        "        m = _orig_init_builtin(name)\n"
		"    if name == 'time' and m is not None:\n"
        "        if not hasattr(m, 'clock'): m.clock = m.time\n"
        "        if not hasattr(m, 'mktime'): m.mktime = _t.mktime\n"
        "        m.clock = m.time\n"
        "    return m\n"
        "imp.init_builtin = _fast_init_builtin\n"
        "_orig_import = __builtin__.__import__\n"
        "def _fast_import(name, globals=None, locals=None, fromlist=None, level=-1):\n"
        "    if '.' not in name:\n"
        "        m = sys.modules.get(name)\n"
        "        if m is not None and not hasattr(m, '__path__'):\n"
        "            return m\n"
        "    return _orig_import(name, globals, locals, fromlist)\n"
        "__builtin__.__import__ = _fast_import\n"
    );

    PyEval_SetProfile(MobilePyProfileTracer, NULL); g_szCrashStage = "Engine Ready / Render Loop"; s_bEngineReady = true;
    Tracen("Metin2 Mobile: Engine Initialized Successfully!");
    LOGI("Metin2 Mobile Engine Initialized Successfully");
}

struct AndroidTouchEvt {
    int pointerId;
    int action;
    int x;
    int y;
};
static AndroidTouchEvt s_touchQueue[256];
static volatile int s_touchHead = 0;
static volatile int s_touchTail = 0;
static pthread_mutex_t s_inputQueueMutex = PTHREAD_MUTEX_INITIALIZER;

struct AndroidKeyEvt { int keyCode; int unicodeChar; };
static AndroidKeyEvt s_keyQueue[64];
static volatile int s_keyHead = 0;
static volatile int s_keyTail = 0;

struct CAndroidPlayerAccessor : public CPythonPlayer {
    using CPythonPlayer::NEW_MoveToDirection;
    using CPythonPlayer::__ClearAutoAttackTargetActorID;
    using CPythonPlayer::__ClearReservedAction;
    using CPythonPlayer::m_ppyGameWindow;
};

#ifdef ENABLE_OPENGL
#include "../eterlib/Camera.h"

// ============================================================================
// Native C++ Procedural 32-bit RGBA TGA Generator (Saved in whitelisted mark/)
// ============================================================================
static inline uint8_t ClampU8(float v) {
    if (v <= 0.0f) return 0;
    if (v >= 255.0f) return 255;
    return (uint8_t)(v + 0.5f);
}

static inline float SmoothStepF(float e0, float e1, float x) {
    if (e1 == e0) return 0.0f;
    float t = (x - e0) / (e1 - e0);
    if (t < 0.0f) t = 0.0f;
    else if (t > 1.0f) t = 1.0f;
    return t * t * (3.0f - 2.0f * t);
}

static inline float DistToSegF(float px, float py, float x1, float y1, float x2, float y2) {
    float vx = x2 - x1, vy = y2 - y1;
    float wx = px - x1, wy = py - y1;
    float c1 = vx * wx + vy * wy;
    if (c1 <= 0.0f) return hypotf(px - x1, py - y1);
    float c2 = vx * vx + vy * vy;
    if (c2 <= c1) return hypotf(px - x2, py - y2);
    float b = c1 / c2;
    return hypotf(px - (x1 + b * vx), py - (y1 + b * vy));
}

enum EMobileIconType {
    MICON_NONE = 0,
    MICON_JOY_BG = 1,
    MICON_JOY_KNOB = 2,
    MICON_ATK_SWORDS = 3,
    MICON_TARGET = 4,
    MICON_LOOT = 5,
    MICON_PAGE = 6,
    MICON_MOUNT = 7,
    MICON_CHAT_BUBBLE = 8,
    MICON_PLUS = 9,
    MICON_MINUS = 10,
    MICON_MAP_PIN = 11
};

static void WriteMobileRingTGA(const char* path, int size, float outerR, float rimW, float glassAlpha,
                               float rr, float rg, float rb, float ar, float ag, float ab,
                               int iconType, bool isPressed) {
    if (access(path, F_OK) == 0)
        return;
    FILE* fp = fopen(path, "wb");
    if (!fp) return;

    uint8_t hdr[18] = {0};
    hdr[2] = 2;
    hdr[12] = (uint8_t)(size & 0xFF);
    hdr[13] = (uint8_t)((size >> 8) & 0xFF);
    hdr[14] = (uint8_t)(size & 0xFF);
    hdr[15] = (uint8_t)((size >> 8) & 0xFF);
    hdr[16] = 32;
    hdr[17] = 0x28;
    fwrite(hdr, 1, 18, fp);

    std::vector<uint8_t> pix((size_t)size * (size_t)size * 4);
    float half = size * 0.5f;
    size_t idx = 0;

    for (int y = 0; y < size; ++y) {
        float py = (y + 0.5f) - half;
        float ny = py / outerR;
        for (int x = 0; x < size; ++x) {
            float px = (x + 0.5f) - half;
            float nx = px / outerR;
            float d = hypotf(px, py);

            float r = 12.0f, g = 16.0f, b = 24.0f, a = 0.0f;

            if (d < outerR + 3.5f) {
                float sh = 1.0f - SmoothStepF(outerR - 1.0f, outerR + 3.5f, d);
                if (isPressed) {
                    r = ar * 0.65f; g = ag * 0.65f; b = ab * 0.65f;
                    a = sh * 125.0f;
                } else {
                    a = sh * 85.0f;
                }
            }

            float innerEdge = outerR - rimW;
            if (d <= outerR) {
                float bodyMask = 1.0f - SmoothStepF(outerR - 1.2f, outerR + 0.4f, d);
                float radRatio = d / (innerEdge > 1.0f ? innerEdge : 1.0f);
                float sheen = fmaxf(0.0f, -0.55f * nx - 0.65f * ny);
                if (isPressed) {
                    r = 28.0f + ar * 0.28f + sheen * 40.0f;
                    g = 34.0f + ag * 0.28f + sheen * 44.0f;
                    b = 46.0f + ab * 0.28f + sheen * 50.0f;
                    a = fmaxf(a, bodyMask * fminf(235.0f, glassAlpha + 50.0f));
                } else {
                    float vig = 0.70f + 0.30f * (radRatio * radRatio);
                    r = (14.0f + sheen * 28.0f) * vig;
                    g = (19.0f + sheen * 34.0f) * vig;
                    b = (28.0f + sheen * 44.0f) * vig;
                    a = fmaxf(a, bodyMask * glassAlpha);
                }
            }

            float rimDist = fabsf(d - (outerR - rimW * 0.5f));
            if (rimDist < rimW * 0.70f) {
                float rimMask = 1.0f - SmoothStepF(rimW * 0.25f, rimW * 0.70f, rimDist);
                float lGrad = 0.65f + 0.35f * (-0.6f * nx - 0.8f * ny);
                float cr = isPressed ? (ar * 0.9f + 25.0f) : (rr * lGrad);
                float cg = isPressed ? (ag * 0.9f + 25.0f) : (rg * lGrad);
                float cb = isPressed ? (ab * 0.9f + 25.0f) : (rb * lGrad);
                r = r * (1.0f - rimMask) + cr * rimMask;
                g = g * (1.0f - rimMask) + cg * rimMask;
                b = b * (1.0f - rimMask) + cb * rimMask;
                a = fmaxf(a, rimMask * 245.0f);
            }

            float irDist = fabsf(d - (outerR - rimW - 3.2f));
            if (irDist < 1.4f) {
                float irMask = (1.0f - irDist / 1.4f) * 0.45f;
                r = r * (1.0f - irMask) + ar * irMask;
                g = g * (1.0f - irMask) + ag * irMask;
                b = b * (1.0f - irMask) + ab * irMask;
                a = fmaxf(a, irMask * 185.0f);
            }

            float iconMask = 0.0f;
            if (iconType == MICON_JOY_BG) {
                float recDist = fabsf(d - outerR * 0.42f);
                if (recDist < 1.4f) iconMask = (1.0f - recDist / 1.4f) * 0.38f;
                const float angles[4] = { 0.0f, 1.570796f, 3.141592f, 4.712389f };
                for (int k = 0; k < 4; ++k) {
                    float ca = cosf(angles[k]), sa = sinf(angles[k]);
                    float tx = px * ca + py * sa;
                    float ty = -px * sa + py * ca;
                    if (tx >= outerR * 0.66f && tx <= outerR * 0.82f) {
                        float span = (outerR * 0.82f - tx) * 0.65f;
                        float edge = fabsf(ty) - span;
                        if (edge < 1.2f)
                            iconMask = fmaxf(iconMask, (1.0f - SmoothStepF(-0.4f, 1.2f, edge)) * 0.82f);
                    }
                }
            } else if (iconType == MICON_JOY_KNOB) {
                float cRing = fabsf(d - outerR * 0.48f);
                if (cRing < 1.9f) iconMask = (1.0f - cRing / 1.9f) * (isPressed ? 0.90f : 0.60f);
                if (d < outerR * 0.24f)
                    iconMask = fmaxf(iconMask, (1.0f - SmoothStepF(outerR * 0.08f, outerR * 0.24f, d)) * 0.95f);
            } else if (iconType == MICON_ATK_SWORDS) {
                float s = outerR / 52.0f;
                float dB1 = DistToSegF(px, py, -18.0f * s, 18.0f * s, 21.0f * s, -21.0f * s);
                float dG1 = DistToSegF(px, py, -19.0f * s, 5.0f * s, -5.0f * s, 19.0f * s);
                float dB2 = DistToSegF(px, py, 18.0f * s, 18.0f * s, -21.0f * s, -21.0f * s);
                float dG2 = DistToSegF(px, py, 5.0f * s, 19.0f * s, 19.0f * s, 5.0f * s);
                float minS = fminf(fminf(dB1, dB2), fminf(dG1, dG2) * 1.15f);
                if (minS < 3.3f * s)
                    iconMask = 1.0f - SmoothStepF(1.4f * s, 3.3f * s, minS);
            } else if (iconType == MICON_TARGET) {
                float s = outerR / 28.0f;
                float rRing = fabsf(d - 11.0f * s);
                if (rRing < 1.9f * s) iconMask = 1.0f - SmoothStepF(0.6f * s, 1.9f * s, rRing);
                if ((fabsf(px) < 1.6f * s && fabsf(py) >= 6.5f * s && fabsf(py) <= 15.5f * s) ||
                    (fabsf(py) < 1.6f * s && fabsf(px) >= 6.5f * s && fabsf(px) <= 15.5f * s))
                    iconMask = fmaxf(iconMask, 0.92f);
                if (d < 2.6f * s)
                    iconMask = fmaxf(iconMask, 1.0f - SmoothStepF(1.0f * s, 2.6f * s, d));
            } else if (iconType == MICON_LOOT) {
                float s = outerR / 28.0f;
                float starV = powf(fabsf(px), 0.65f) + powf(fabsf(py + 2.0f * s), 0.65f);
                if (starV < powf(10.5f * s, 0.65f)) iconMask = 0.95f;
                float arcD = fabsf(hypotf(px, py - 2.0f * s) - 12.5f * s);
                if (py > 1.0f * s && arcD < 2.0f * s)
                    iconMask = fmaxf(iconMask, 1.0f - SmoothStepF(0.7f * s, 2.0f * s, arcD));
            } else if (iconType == MICON_PAGE) {
                float s = outerR / 28.0f;
                float orbD = fabsf(d - 12.5f * s);
                if (orbD < 2.0f * s && fabsf(px) > 2.5f * s)
                    iconMask = 1.0f - SmoothStepF(0.7f * s, 2.0f * s, orbD);
            } else if (iconType == MICON_MOUNT) {
                float s = outerR / 28.0f;
                float shoeDist = fabsf(hypotf(px, py - 1.0f * s) - 11.5f * s);
                if (shoeDist < 2.6f * s && !(py < -4.5f * s && fabsf(px) < 6.5f * s))
                    iconMask = 1.0f - SmoothStepF(1.1f * s, 2.6f * s, shoeDist);
            } else if (iconType == MICON_CHAT_BUBBLE) {
                float s = outerR / 28.0f;
                float bx = px / (12.0f * s);
                float by = (py + 2.0f * s) / (9.5f * s);
                float bd = hypotf(bx, by);
                if (bd <= 1.0f) {
                    iconMask = 1.0f - SmoothStepF(0.82f, 1.0f, bd);
                    if (fabsf(py + 2.0f * s) < 1.3f * s && (fabsf(px) < 1.4f * s || fabsf(fabsf(px) - 5.2f * s) < 1.4f * s))
                        iconMask = 0.15f;
                }
            } else if (iconType == MICON_PLUS) {
                float s = outerR / 14.0f;
                if ((fabsf(px) < 1.6f * s && fabsf(py) < 6.5f * s) || (fabsf(py) < 1.6f * s && fabsf(px) < 6.5f * s))
                    iconMask = 0.95f;
            } else if (iconType == MICON_MINUS) {
                float s = outerR / 14.0f;
                if (fabsf(py) < 1.6f * s && fabsf(px) < 6.5f * s)
                    iconMask = 0.95f;
            } else if (iconType == MICON_MAP_PIN) {
                float s = outerR / 22.0f;
                float ringD = fabsf(hypotf(px, py + 3.0f * s) - 5.5f * s);
                if (ringD < 1.8f * s) iconMask = 1.0f - SmoothStepF(0.6f * s, 1.8f * s, ringD);
                float tailD = DistToSegF(px, py, 0.0f, 3.0f * s, 0.0f, 9.5f * s);
                if (tailD < 2.2f * s) iconMask = fmaxf(iconMask, 1.0f - SmoothStepF(0.8f * s, 2.2f * s, tailD));
            }

            if (iconMask > 0.0f) {
                float icR = isPressed ? 255.0f : (ar * 0.55f + 255.0f * 0.45f);
                float icG = isPressed ? 246.0f : (ag * 0.55f + 235.0f * 0.45f);
                float icB = isPressed ? 215.0f : (ab * 0.55f + 190.0f * 0.45f);
                r = r * (1.0f - iconMask) + icR * iconMask;
                g = g * (1.0f - iconMask) + icG * iconMask;
                b = b * (1.0f - iconMask) + icB * iconMask;
                a = fmaxf(a, iconMask * 250.0f);
            }

            pix[idx + 0] = ClampU8(b);
            pix[idx + 1] = ClampU8(g);
            pix[idx + 2] = ClampU8(r);
            pix[idx + 3] = ClampU8(a);
            idx += 4;
        }
    }

    fwrite(pix.data(), 1, pix.size(), fp);
    fclose(fp);
}

static void WriteSquaredMiniMapTextures() {
    // 1. Squared Radar Filter Mask (128x128) for CPythonMiniMap
    if (access("mark/m_minimap_mask.tga", F_OK) != 0) {
        FILE* fp = fopen("mark/m_minimap_mask.tga", "wb");
        if (fp) {
            uint8_t hdr[18] = {0};
            hdr[2] = 2; hdr[12] = 128; hdr[14] = 128; hdr[16] = 32; hdr[17] = 0x28;
            fwrite(hdr, 1, 18, fp);
            std::vector<uint8_t> pix(128 * 128 * 4, 255);
            for (int y = 0; y < 128; ++y) {
                for (int x = 0; x < 128; ++x) {
                    int edgeDist = std::min(std::min(x, 127 - x), std::min(y, 127 - y));
                    uint8_t a = (edgeDist <= 1) ? 0 : (edgeDist == 2 ? 180 : 255);
                    size_t i = ((size_t)y * 128 + (size_t)x) * 4;
                    pix[i + 0] = 255; pix[i + 1] = 255; pix[i + 2] = 255; pix[i + 3] = a;
                }
            }
            fwrite(pix.data(), 1, pix.size(), fp);
            fclose(fp);
        }
    }

    // 2. Royal2 Squared Brushed-Gold & Obsidian Frame Overlay (136x136)
    if (access("mark/m_minimap_frame.tga", F_OK) != 0) {
        const int sz = 136;
        FILE* fp = fopen("mark/m_minimap_frame.tga", "wb");
        if (fp) {
            uint8_t hdr[18] = {0};
            hdr[2] = 2; hdr[12] = sz; hdr[14] = sz; hdr[16] = 32; hdr[17] = 0x28;
            fwrite(hdr, 1, 18, fp);
            std::vector<uint8_t> pix((size_t)sz * (size_t)sz * 4, 0);
            for (int y = 0; y < sz; ++y) {
                for (int x = 0; x < sz; ++x) {
                    int d = std::min(std::min(x, sz - 1 - x), std::min(y, sz - 1 - y));
                    size_t i = ((size_t)y * sz + (size_t)x) * 4;
                    if (d < 6) {
                        // Outer shadow + double metallic gold bevel border
                        if (d == 0) {
                            pix[i+0] = 10; pix[i+1] = 12; pix[i+2] = 16; pix[i+3] = 180;
                        } else if (d >= 1 && d <= 3) {
                            float grad = 0.72f + 0.28f * (1.0f - (float)(x + y) / (2.0f * sz));
                            pix[i+0] = ClampU8(98.0f * grad);
                            pix[i+1] = ClampU8(178.0f * grad);
                            pix[i+2] = ClampU8(218.0f * grad);
                            pix[i+3] = 245;
                        } else if (d == 4) {
                            pix[i+0] = 24; pix[i+1] = 28; pix[i+2] = 36; pix[i+3] = 220;
                        } else if (d == 5) {
                            pix[i+0] = 75; pix[i+1] = 135; pix[i+2] = 170; pix[i+3] = 160;
                        }
                    }
                }
            }
            fwrite(pix.data(), 1, pix.size(), fp);
            fclose(fp);
        }
    }
}

enum ETopBarIcon {
    TICON_COLLAPSE = 1,
    TICON_EXPAND = 2,
    TICON_CHAR = 3,
    TICON_INVEN = 4,
    TICON_COMM = 5,
    TICON_SHOP = 6,
    TICON_AUTOHUNT = 7,
    TICON_DS = 8,
    TICON_UTIL = 9,
    TICON_SYS = 10
};

static void WriteTopBarButtonTGA(const char* path, int iconType, bool isPressed) {
    if (access(path, F_OK) == 0)
        return;

    const int size = 40;
    FILE* fp = fopen(path, "wb");
    if (!fp) return;

    uint8_t hdr[18] = {0};
    hdr[2] = 2; hdr[12] = size; hdr[14] = size; hdr[16] = 32; hdr[17] = 0x28;
    fwrite(hdr, 1, 18, fp);

    std::vector<uint8_t> pix((size_t)size * (size_t)size * 4, 0);
    float half = size * 0.5f;
    size_t idx = 0;

    for (int y = 0; y < size; ++y) {
        float py = (y + 0.5f) - half;
        for (int x = 0; x < size; ++x) {
            float px = (x + 0.5f) - half;
            // Chamfered octagonal-square distance (distinct from circular buttons)
            float ax = fabsf(px), ay = fabsf(py);
            float octDist = fmaxf(fmaxf(ax, ay), (ax + ay) * 0.76f);
            float outerR = 17.5f;

            float r = 10.0f, g = 12.0f, b = 16.0f, a = 0.0f;

            // 1. Soft dark outer shadow
            if (octDist < outerR + 2.0f) {
                float sh = 1.0f - SmoothStepF(outerR - 0.5f, outerR + 2.0f, octDist);
                a = sh * 110.0f;
            }

            // 2. Matte-black / obsidian interior
            if (octDist <= outerR) {
                float bodyMask = 1.0f - SmoothStepF(outerR - 1.0f, outerR + 0.3f, octDist);
                float sheen = fmaxf(0.0f, (-px - py) / (2.0f * outerR));
                if (isPressed) {
                    r = 48.0f + sheen * 45.0f;
                    g = 54.0f + sheen * 48.0f;
                    b = 66.0f + sheen * 55.0f;
                    a = fmaxf(a, bodyMask * 235.0f);
                } else {
                    r = 14.0f + sheen * 22.0f;
                    g = 16.0f + sheen * 24.0f;
                    b = 20.0f + sheen * 28.0f;
                    a = fmaxf(a, bodyMask * 205.0f);
                }
            }

            // 3. Crisp White/Silver double-bevel border
            float rimDist = fabsf(octDist - (outerR - 1.4f));
            if (rimDist < 1.5f) {
                float rimMask = 1.0f - SmoothStepF(0.4f, 1.5f, rimDist);
                float silver = isPressed ? 255.0f : (215.0f + 35.0f * (1.0f - (float)(x + y) / (2.0f * size)));
                r = r * (1.0f - rimMask) + silver * rimMask;
                g = g * (1.0f - rimMask) + silver * rimMask;
                b = b * (1.0f - rimMask) + fminf(255.0f, silver + 8.0f) * rimMask;
                a = fmaxf(a, rimMask * 248.0f);
            }

            // 4. Pure White Vector Glyph
            float ic = 0.0f;
            if (iconType == TICON_COLLAPSE) {
                // '>' Chevron
                float d1 = DistToSegF(px, py, -3.5f, -6.5f, 4.0f, 0.0f);
                float d2 = DistToSegF(px, py, 4.0f, 0.0f, -3.5f, 6.5f);
                float md = fminf(d1, d2);
                if (md < 2.0f) ic = 1.0f - SmoothStepF(0.8f, 2.0f, md);
            } else if (iconType == TICON_EXPAND) {
                // '<' Chevron
                float d1 = DistToSegF(px, py, 3.5f, -6.5f, -4.0f, 0.0f);
                float d2 = DistToSegF(px, py, -4.0f, 0.0f, 3.5f, 6.5f);
                float md = fminf(d1, d2);
                if (md < 2.0f) ic = 1.0f - SmoothStepF(0.8f, 2.0f, md);
            } else if (iconType == TICON_SYS) {
                // Hamburger Menu (3 horizontal bars)
                if (ax < 7.5f && (fabsf(py + 5.5f) < 1.3f || fabsf(py) < 1.3f || fabsf(py - 5.5f) < 1.3f))
                    ic = 0.96f;
            } else if (iconType == TICON_CHAR) {
                // Character / Crowned Helmet silhouette
                float headD = hypotf(px, py + 4.2f);
                if (headD < 3.6f) ic = 1.0f - SmoothStepF(2.6f, 3.6f, headD);
                float shD = hypotf(px * 0.75f, py - 4.8f);
                if (py > 0.5f && py < 8.5f && shD < 6.2f)
                    ic = fmaxf(ic, 1.0f - SmoothStepF(5.0f, 6.2f, shD));
            } else if (iconType == TICON_INVEN) {
                // Inventory Chest / Bag
                if (ax < 7.0f && py > -4.5f && py < 7.5f) {
                    float edge = fmaxf(ax - 5.5f, fmaxf(-py - 3.0f, py - 6.0f));
                    if (edge > -1.8f || fabsf(py + 0.5f) < 1.1f) ic = 0.95f;
                }
                float hnd = fabsf(hypotf(px, py + 4.5f) - 3.5f);
                if (py <= -4.5f && hnd < 1.4f) ic = fmaxf(ic, 0.92f);
            } else if (iconType == TICON_COMM) {
                // Community / 2 People
                float h1 = hypotf(px + 3.2f, py + 3.8f);
                float h2 = hypotf(px - 3.8f, py + 2.8f);
                if (h1 < 2.8f || h2 < 2.4f) ic = 0.95f;
                if (py > 0.5f && py < 7.5f) {
                    if (hypotf((px + 3.2f) * 0.8f, py - 4.5f) < 4.5f || hypotf((px - 3.8f) * 0.85f, py - 5.0f) < 3.8f)
                        ic = 0.95f;
                }
            } else if (iconType == TICON_SHOP) {
                // Shop / Storefront / Scales
                if (ax < 7.5f && py >= -6.5f && py <= -2.5f) ic = 0.95f;
                if (ax < 6.2f && py > -1.5f && py < 7.5f && (ax > 4.4f || py > 5.5f)) ic = 0.95f;
            } else if (iconType == TICON_AUTOHUNT) {
                // Crossed Swords (like Royal2 top bar)
                float d1 = DistToSegF(px, py, -6.5f, 6.5f, 6.5f, -6.5f);
                float d2 = DistToSegF(px, py, 6.5f, 6.5f, -6.5f, -6.5f);
                float md = fminf(d1, d2);
                if (md < 1.7f) ic = 1.0f - SmoothStepF(0.7f, 1.7f, md);
            } else if (iconType == TICON_DS) {
                // Dragon Soul Diamond
                float dia = ax * 1.15f + ay * 0.85f;
                if (dia < 8.0f) {
                    if (dia > 5.6f || dia < 3.0f) ic = 1.0f - SmoothStepF(7.2f, 8.0f, dia);
                }
            } else if (iconType == TICON_UTIL) {
                // 4-Square Utility Grid
                if (ax >= 1.8f && ax <= 6.8f && ay >= 1.8f && ay <= 6.8f)
                    ic = 0.95f;
            }

            if (ic > 0.0f) {
                r = r * (1.0f - ic) + 255.0f * ic;
                g = g * (1.0f - ic) + 255.0f * ic;
                b = b * (1.0f - ic) + 255.0f * ic;
                a = fmaxf(a, ic * 255.0f);
            }

            pix[idx + 0] = ClampU8(b);
            pix[idx + 1] = ClampU8(g);
            pix[idx + 2] = ClampU8(r);
            pix[idx + 3] = ClampU8(a);
            idx += 4;
        }
    }

    fwrite(pix.data(), 1, pix.size(), fp);
    fclose(fp);
}

static void WritePortraitRingTGA(const char* path) {
    if (access(path, F_OK) == 0)
        return;
    const int size = 64;
    FILE* fp = fopen(path, "wb");
    if (!fp) return;

    uint8_t hdr[18] = {0};
    hdr[2] = 2; hdr[12] = size; hdr[14] = size; hdr[16] = 32; hdr[17] = 0x28;
    fwrite(hdr, 1, 18, fp);

    std::vector<uint8_t> pix((size_t)size * (size_t)size * 4, 0);
    float half = size * 0.5f;
    float outerR = 29.5f;
    float innerR = 23.0f;

    for (int y = 0; y < size; ++y) {
        float py = (y + 0.5f) - half;
        for (int x = 0; x < size; ++x) {
            float px = (x + 0.5f) - half;
            float d = hypotf(px, py);
            size_t i = ((size_t)y * size + (size_t)x) * 4;

            float r = 12.0f, g = 15.0f, b = 22.0f, a = 0.0f;

            // Soft outer drop shadow
            if (d >= innerR && d < outerR + 2.5f) {
                float sh = 1.0f - SmoothStepF(outerR - 0.5f, outerR + 2.5f, d);
                a = sh * 120.0f;
            }
            // Brushed metallic gold & silver double bezel around the face window
            if (d >= innerR - 1.0f && d <= outerR) {
                float inMask  = SmoothStepF(innerR - 1.0f, innerR + 0.8f, d);
                float outMask = 1.0f - SmoothStepF(outerR - 1.0f, outerR + 0.4f, d);
                float ringMask = inMask * outMask;

                float grad = 0.65f + 0.35f * (1.0f - (float)(x + y) / (2.0f * size));
                bool isInnerLip = (d < innerR + 1.6f);
                if (isInnerLip) {
                    r = 235.0f * grad; g = 240.0f * grad; b = 250.0f * grad;
                } else {
                    r = 222.0f * grad; g = 184.0f * grad; b = 105.0f * grad;
                }
                a = fmaxf(a, ringMask * 250.0f);
            }

            pix[i + 0] = ClampU8(b);
            pix[i + 1] = ClampU8(g);
            pix[i + 2] = ClampU8(r);
            pix[i + 3] = ClampU8(a);
        }
    }
    fwrite(pix.data(), 1, pix.size(), fp);
    fclose(fp);
}

static void WriteEnergyCrystalTGA(const char* path, float cr, float cg, float cb) {
    if (access(path, F_OK) == 0)
        return;
    const int size = 32;
    FILE* fp = fopen(path, "wb");
    if (!fp) return;

    uint8_t hdr[18] = {0};
    hdr[2] = 2; hdr[12] = size; hdr[14] = size; hdr[16] = 32; hdr[17] = 0x28;
    fwrite(hdr, 1, 18, fp);

    std::vector<uint8_t> pix((size_t)size * (size_t)size * 4, 0);
    float half = size * 0.5f;
    float outerR = 13.5f;

    for (int y = 0; y < size; ++y) {
        float py = (y + 0.5f) - half;
        for (int x = 0; x < size; ++x) {
            float px = (x + 0.5f) - half;
            float d = hypotf(px, py);
            size_t i = ((size_t)y * size + (size_t)x) * 4;

            float r = 14.0f, g = 18.0f, b = 26.0f, a = 0.0f;
            if (d <= outerR + 1.5f) {
                float mask = 1.0f - SmoothStepF(outerR - 0.8f, outerR + 1.2f, d);
                a = mask * 215.0f;

                // Faceted diamond crystal core
                float dia = fabsf(px) * 1.25f + fabsf(py) * 0.9f;
                if (dia < 8.5f) {
                    float coreGlow = 1.0f - (dia / 8.5f);
                    float spec = (px < 0.0f && py < 0.0f) ? 55.0f : 0.0f;
                    r = fminf(255.0f, cr * (0.55f + 0.65f * coreGlow) + spec);
                    g = fminf(255.0f, cg * (0.55f + 0.65f * coreGlow) + spec);
                    b = fminf(255.0f, cb * (0.55f + 0.65f * coreGlow) + spec);
                    a = 248.0f;
                }

                // Brushed gold outer rim
                float rimDist = fabsf(d - (outerR - 1.2f));
                if (rimDist < 1.4f) {
                    float rm = 1.0f - rimDist / 1.4f;
                    r = r * (1.0f - rm) + 220.0f * rm;
                    g = g * (1.0f - rm) + 185.0f * rm;
                    b = b * (1.0f - rm) + 110.0f * rm;
                    a = fmaxf(a, rm * 250.0f);
                }
            }
            pix[i + 0] = ClampU8(b);
            pix[i + 1] = ClampU8(g);
            pix[i + 2] = ClampU8(r);
            pix[i + 3] = ClampU8(a);
        }
    }
    fwrite(pix.data(), 1, pix.size(), fp);
    fclose(fp);
}

static void WriteMobileStatusHUDTextures() {
    WriteMobileRingTGA("mark/m_portrait_bg.tga", 64, 28.5f, 2.2f, 195.0f, 218,182,108, 110,210,255, MICON_NONE, false);
    WritePortraitRingTGA("mark/m_portrait_ring.tga");
    WriteEnergyCrystalTGA("mark/m_energy_empty.tga",   95.0f, 105.0f, 120.0f);
    WriteEnergyCrystalTGA("mark/m_energy_hungry.tga", 255.0f, 165.0f,  40.0f);
    WriteEnergyCrystalTGA("mark/m_energy_full.tga",    65.0f, 205.0f, 255.0f);
}

void GenerateMobileHUDTextures() {
    // Bump texture names to v2 so larger, high-visibility sizes are generated cleanly
    WriteMobileRingTGA("mark/m_joy_bg_v2.tga",        160, 74.0f, 3.8f,  98.0f, 218,182,108, 110,210,255, MICON_JOY_BG,      false);
    WriteMobileRingTGA("mark/m_joy_knob_norm_v2.tga",  76, 33.0f, 3.2f, 165.0f, 218,182,108, 255,205, 95, MICON_JOY_KNOB,    false);
    WriteMobileRingTGA("mark/m_joy_knob_down_v2.tga",  76, 35.0f, 3.8f, 205.0f, 110,210,255, 110,210,255, MICON_JOY_KNOB,    true);

    WriteMobileRingTGA("mark/m_atk_norm_v2.tga",      140, 63.0f, 4.5f, 148.0f, 218,182,108, 255,205, 95, MICON_ATK_SWORDS,  false);
    WriteMobileRingTGA("mark/m_atk_down_v2.tga",      140, 65.0f, 5.2f, 198.0f, 255,205, 95, 110,210,255, MICON_ATK_SWORDS,  true);

    WriteMobileRingTGA("mark/m_slot_norm_v2.tga",      72, 33.0f, 3.0f, 128.0f, 218,182,108, 165,185,210, MICON_NONE,        false);
    WriteMobileRingTGA("mark/m_slot_over_v2.tga",      72, 34.0f, 3.5f, 172.0f, 255,205, 95, 110,210,255, MICON_NONE,        true);

    WriteMobileRingTGA("mark/m_btn_lock_norm.tga",     64, 26.0f, 2.8f, 132.0f, 165,185,210, 110,210,255, MICON_TARGET,      false);
    WriteMobileRingTGA("mark/m_btn_lock_down.tga",     64, 27.0f, 3.2f, 188.0f, 110,210,255, 110,210,255, MICON_TARGET,      true);
    WriteMobileRingTGA("mark/m_btn_loot_norm.tga",     64, 26.0f, 2.8f, 132.0f, 218,182,108, 255,205, 95, MICON_LOOT,        false);
    WriteMobileRingTGA("mark/m_btn_loot_down.tga",     64, 27.0f, 3.2f, 188.0f, 255,205, 95, 255,205, 95, MICON_LOOT,        true);
    WriteMobileRingTGA("mark/m_btn_page_norm.tga",     64, 26.0f, 2.8f, 132.0f, 218,182,108, 110,210,255, MICON_PAGE,        false);
    WriteMobileRingTGA("mark/m_btn_page_down.tga",     64, 27.0f, 3.2f, 188.0f, 110,210,255, 110,210,255, MICON_PAGE,        true);
    WriteMobileRingTGA("mark/m_btn_mount_norm.tga",    64, 28.0f, 3.0f, 132.0f, 218,182,108, 255,205, 95, MICON_MOUNT,       false);
    WriteMobileRingTGA("mark/m_btn_mount_down.tga",    64, 29.0f, 3.4f, 188.0f, 255,205, 95, 255,205, 95, MICON_MOUNT,       true);

    WriteMobileRingTGA("mark/m_btn_chat_norm.tga",     56, 24.0f, 2.6f, 138.0f, 218,182,108, 110,210,255, MICON_CHAT_BUBBLE, false);
    WriteMobileRingTGA("mark/m_btn_chat_down.tga",     56, 25.0f, 3.0f, 192.0f, 110,210,255, 110,210,255, MICON_CHAT_BUBBLE, true);

    WriteMobileRingTGA("mark/m_mm_plus_norm.tga",      28, 12.0f, 2.0f, 155.0f, 218,182,108, 255,205, 95, MICON_PLUS,        false);
    WriteMobileRingTGA("mark/m_mm_plus_down.tga",      28, 12.5f, 2.2f, 200.0f, 110,210,255, 110,210,255, MICON_PLUS,        true);
    WriteMobileRingTGA("mark/m_mm_minus_norm.tga",     28, 12.0f, 2.0f, 155.0f, 218,182,108, 255,205, 95, MICON_MINUS,       false);
    WriteMobileRingTGA("mark/m_mm_minus_down.tga",     28, 12.5f, 2.2f, 200.0f, 110,210,255, 110,210,255, MICON_MINUS,       true);
    WriteMobileRingTGA("mark/m_mm_atlas_norm.tga",     42, 18.5f, 2.4f, 145.0f, 218,182,108, 110,210,255, MICON_MAP_PIN,     false);
    WriteMobileRingTGA("mark/m_mm_atlas_down.tga",     42, 19.5f, 2.8f, 195.0f, 110,210,255, 110,210,255, MICON_MAP_PIN,     true);
	WriteTopBarButtonTGA("mark/m_top_collapse_norm.tga", TICON_COLLAPSE, false);
    WriteTopBarButtonTGA("mark/m_top_collapse_down.tga", TICON_COLLAPSE, true);
    WriteTopBarButtonTGA("mark/m_top_expand_norm.tga",   TICON_EXPAND,   false);
    WriteTopBarButtonTGA("mark/m_top_expand_down.tga",   TICON_EXPAND,   true);
    WriteTopBarButtonTGA("mark/m_top_char_norm.tga",     TICON_CHAR,     false);
    WriteTopBarButtonTGA("mark/m_top_char_down.tga",     TICON_CHAR,     true);
    WriteTopBarButtonTGA("mark/m_top_inven_norm.tga",    TICON_INVEN,    false);
    WriteTopBarButtonTGA("mark/m_top_inven_down.tga",    TICON_INVEN,    true);
    WriteTopBarButtonTGA("mark/m_top_comm_norm.tga",     TICON_COMM,     false);
    WriteTopBarButtonTGA("mark/m_top_comm_down.tga",     TICON_COMM,     true);
    WriteTopBarButtonTGA("mark/m_top_shop_norm.tga",     TICON_SHOP,     false);
    WriteTopBarButtonTGA("mark/m_top_shop_down.tga",     TICON_SHOP,     true);
    WriteTopBarButtonTGA("mark/m_top_auto_norm.tga",     TICON_AUTOHUNT, false);
    WriteTopBarButtonTGA("mark/m_top_auto_down.tga",     TICON_AUTOHUNT, true);
    WriteTopBarButtonTGA("mark/m_top_ds_norm.tga",       TICON_DS,       false);
    WriteTopBarButtonTGA("mark/m_top_ds_down.tga",       TICON_DS,       true);
    WriteTopBarButtonTGA("mark/m_top_util_norm.tga",     TICON_UTIL,     false);
    WriteTopBarButtonTGA("mark/m_top_util_down.tga",     TICON_UTIL,     true);
    WriteTopBarButtonTGA("mark/m_top_sys_norm.tga",      TICON_SYS,      false);
    WriteTopBarButtonTGA("mark/m_top_sys_down.tga",      TICON_SYS,      true);
    WriteSquaredMiniMapTextures();
	WriteMobileStatusHUDTextures();
}

// ============================================================================
// Mobile Controls State (360° Joystick + Cinematic Camera + Multi-Touch + Pinch Zoom)
// ============================================================================
enum EMobileTouchRole {
    TOUCH_ROLE_NONE = 0,
    TOUCH_ROLE_JOYSTICK = 1,
    TOUCH_ROLE_UI = 2,
    TOUCH_ROLE_WORLD = 3
};

struct SMobilePointerSlot {
    bool active = false;
    int role = TOUCH_ROLE_NONE;
    int startX = 0, startY = 0;
    int curX = 0, curY = 0;
    int lastX = 0, lastY = 0;
    DWORD downTimeMs = 0;
    bool isDraggingCamera = false;
};

static SMobilePointerSlot s_pointers[4];
static int s_iJoyCenterX = 148;
static int s_iJoyCenterY = 562;
static int s_iJoyRadius  = 84;
static bool s_bJoyActive = false;
static bool s_bJoyMovingPlayer = false;
static float s_fJoyNormX = 0.0f;
static float s_fJoyNormY = 0.0f;
static float s_fJoyRawDx = 0.0f;
static float s_fJoyRawDy = 0.0f;
static float s_fSmoothedAutoRollRate = 0.0f;
static bool s_bUiTouchDown = false;
static DWORD s_dwLastLockedVID = 0;

// Cinematic Camera State
static float s_fCamAccumDx = 0.0f;
static float s_fCamAccumDy = 0.0f;
static float s_fCamVelRoll = 0.0f;
static float s_fCamVelPitch = 0.0f;
static float s_fManualCamCooldown = 0.0f;
static float s_fBaseCamDistance = -1.0f;
static float s_fSmoothedSpeedZoomOffset = 0.0f;
static float s_fLastPinchSpan = -1.0f;

static bool s_bSoftKeyboardVisible = false;
static DWORD s_dwLastKeyboardOpenMs = 0;

void AndroidToggleSoftKeyboard(bool show) {
    if (show) {
        s_dwLastKeyboardOpenMs = ELTimer_GetMSec();
    }
    s_bSoftKeyboardVisible = show;

    // Show or hide the global TOP_MOST live text input bar above the keyboard
    PyRun_SimpleString(show ?
        "try:\n"
        "    import ui, wndMgr, ime\n"
        "    if not hasattr(ui, '_g_mobileKbBar') or ui._g_mobileKbBar is None:\n"
        "        class _MobileKbPreview(ui.Window):\n"
        "            def __init__(self):\n"
        "                ui.Window.__init__(self, 'TOP_MOST')\n"
        "                sw = wndMgr.GetScreenWidth()\n"
        "                self.SetSize(500, 42)\n"
        "                self.SetPosition((sw - 500) / 2, 8)\n"
        "                self.txt = ui.TextLine()\n"
        "                self.txt.SetParent(self)\n"
        "                self.txt.SetPosition(210, 12)\n"
        "                self.txt.SetHorizontalAlignCenter()\n"
        "                self.txt.SetOutline()\n"
        "                self.txt.SetPackedFontColor(0xFFFFFFFF)\n"
        "                self.txt.AddFlag('not_pick')\n"
        "                self.txt.Show()\n"
        "                self.doneTxt = ui.TextLine()\n"
        "                self.doneTxt.SetParent(self)\n"
        "                self.doneTxt.SetPosition(456, 12)\n"
        "                self.doneTxt.SetHorizontalAlignCenter()\n"
        "                self.doneTxt.SetOutline()\n"
        "                self.doneTxt.SetPackedFontColor(0xFF6ED2FF)\n"
        "                self.doneTxt.SetText('تم')\n"
        "                self.doneTxt.AddFlag('not_pick')\n"
        "                self.doneTxt.Show()\n"
        "            def SetPreviewText(self, val, isSecret):\n"
        "                sw = wndMgr.GetScreenWidth()\n"
        "                self.SetPosition((sw - 500) / 2, 8)\n"
        "                disp = ('*' * len(val)) if isSecret else val\n"
        "                self.txt.SetText((disp if disp else '') + ' |')\n"
        "            def OnMouseLeftButtonDown(self):\n"
        "                lx, ly = self.GetMouseLocalPosition()\n"
        "                if lx >= 412:\n"
        "                    ime.PasteReturn()\n"
        "                return True\n"
        "            def OnRender(self):\n"
        "                import grp\n"
        "                x, y = self.GetGlobalPosition()\n"
        "                w, h = self.GetWidth(), self.GetHeight()\n"
        "                grp.SetColor(0xF20A101C)\n"
        "                grp.RenderBar(x, y, w, h)\n"
        "                grp.SetColor(0xFF1D3D5E)\n"
        "                grp.RenderBar(x + w - 84, y + 4, 80, h - 8)\n"
        "                grp.SetColor(0xFF6ED2FF)\n"
        "                grp.RenderBox(x, y, w, h)\n"
        "                grp.RenderBox(x + w - 84, y + 4, 80, h - 8)\n"
        "        ui._g_mobileKbBar = _MobileKbPreview()\n"
        "    ui._g_mobileKbBar.Show()\n"
        "    ui._g_mobileKbBar.SetTop()\n"
        "except:\n"
        "    pass\n"
        :
        "try:\n"
        "    import ui\n"
        "    if hasattr(ui, '_g_mobileKbBar') and ui._g_mobileKbBar:\n"
        "        ui._g_mobileKbBar.Hide()\n"
        "except:\n"
        "    pass\n"
    );

#ifdef __APPLE__
    IOSToggleSoftKeyboard(show);
    return;
#else
    if (!g_pJavaVM) return;
    JNIEnv* env = nullptr;
    if (g_pJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK || !env) return;

    jclass actThreadCls = env->FindClass("android/app/ActivityThread");
    if (!actThreadCls) { env->ExceptionClear(); return; }
    jmethodID curAppMid = env->GetStaticMethodID(actThreadCls, "currentApplication", "()Landroid/app/Application;");
    jobject appObj = curAppMid ? env->CallStaticObjectMethod(actThreadCls, curAppMid) : nullptr;
    if (appObj) {
        jclass ctxCls = env->FindClass("android/content/Context");
        jmethodID getSysMid = env->GetMethodID(ctxCls, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
        jstring immStr = env->NewStringUTF("input_method");
        jobject immObj = env->CallObjectMethod(appObj, getSysMid, immStr);
        if (immObj) {
            jclass immCls = env->GetObjectClass(immObj);
            jmethodID visHMid = env->GetMethodID(immCls, "getInputMethodWindowVisibleHeight", "()I");
            env->ExceptionClear();
            bool shouldToggle = true;
            if (visHMid) {
                jint kbH = env->CallIntMethod(immObj, visHMid);
                if (!env->ExceptionCheck()) {
                    bool isCurrentlyOpen = (kbH > 0);
                    shouldToggle = (isCurrentlyOpen != show);
                }
                env->ExceptionClear();
            }
            if (shouldToggle) {
                jmethodID toggleMid = env->GetMethodID(immCls, "toggleSoftInput", "(II)V");
                if (toggleMid) {
                    env->CallVoidMethod(immObj, toggleMid, show ? 2 : 0, 0);
                }
            }
            env->DeleteLocalRef(immCls);
            env->DeleteLocalRef(immObj);
        }
        env->DeleteLocalRef(immStr);
        env->DeleteLocalRef(ctxCls);
        env->DeleteLocalRef(appObj);
    }
    env->DeleteLocalRef(actThreadCls);
    env->ExceptionClear();
#endif
}

bool AndroidIsSoftKeyboardVisible() {
    return s_bSoftKeyboardVisible;
}

DWORD AndroidGetLastKeyboardOpenMs() {
    return s_dwLastKeyboardOpenMs;
}

static bool IsAnyPointerDraggingCamera() {
    for (int i = 0; i < 4; ++i) {
        if (s_pointers[i].active && s_pointers[i].role == TOUCH_ROLE_WORLD && s_pointers[i].isDraggingCamera)
            return true;
    }
    return false;
}

static int CountActiveWorldPointers(int* outIdx0 = nullptr, int* outIdx1 = nullptr) {
    int count = 0;
    for (int i = 0; i < 4; ++i) {
        if (s_pointers[i].active && s_pointers[i].role == TOUCH_ROLE_WORLD) {
            if (count == 0 && outIdx0) *outIdx0 = i;
            else if (count == 1 && outIdx1) *outIdx1 = i;
            ++count;
        }
    }
    return count;
}

static void UpdateUiTouchDownFlag() {
    bool anyUi = false;
    for (int i = 0; i < 4; ++i) {
        if (s_pointers[i].active && s_pointers[i].role == TOUCH_ROLE_UI) {
            anyUi = true;
            break;
        }
    }
    s_bUiTouchDown = anyUi;
}

static void UpdateJoystickFromPoint(int x, int y) {
    float dx = (float)(x - s_iJoyCenterX);
    float dy = (float)(y - s_iJoyCenterY);
    float dist = sqrtf(dx * dx + dy * dy);
    float maxR = (float)(s_iJoyRadius > 10 ? s_iJoyRadius : 84);

    if (dist > maxR) {
        dx = (dx / dist) * maxR;
        dy = (dy / dist) * maxR;
    }
    s_fJoyRawDx = dx;
    s_fJoyRawDy = dy;
    s_fJoyNormX = dx / maxR;
    s_fJoyNormY = dy / maxR;
}

static void ResetJoystickState() {
    s_bJoyActive = false;
    s_fJoyNormX = 0.0f;
    s_fJoyNormY = 0.0f;
    s_fJoyRawDx = 0.0f;
    s_fJoyRawDy = 0.0f;
    if (s_bJoyMovingPlayer && CPythonPlayer::InstancePtr()) {
        CInstanceBase* pMain = CPythonCharacterManager::InstancePtr() ? CPythonCharacterManager::Instance().GetMainInstancePtr() : nullptr;
        if (!pMain || (!pMain->IsAttacking() && !pMain->IsUsingSkill())) {
            CPythonPlayer::Instance().NEW_Stop();
        }
        s_bJoyMovingPlayer = false;
    }
}

static int FindBestActiveSlotForPoint(int pidHint, int x, int y, bool forMove) {
    int activeCount = 0;
    int onlyIdx = -1;
    for (int i = 0; i < 4; ++i) {
        if (s_pointers[i].active) {
            ++activeCount;
            onlyIdx = i;
        }
    }
    if (activeCount == 0) return -1;
    if (activeCount == 1) return onlyIdx;

    // Multiple fingers are active: match by proximity to current finger position
    int bestIdx = -1;
    int bestDistSq = 0x7FFFFFFF;
    for (int i = 0; i < 4; ++i) {
        if (!s_pointers[i].active) continue;
        // Prevent right-side button touches from hijacking the left-side joystick during multi-touch
        if (forMove && s_pointers[i].role == TOUCH_ROLE_JOYSTICK && x > (g_AndroidScreenWidth / 2))
            continue;
        int dx = x - s_pointers[i].curX;
        int dy = y - s_pointers[i].curY;
        int dSq = dx * dx + dy * dy;
        if (dSq < bestDistSq) {
            bestDistSq = dSq;
            bestIdx = i;
        }
    }
    if (bestIdx >= 0) return bestIdx;
    if (pidHint >= 0 && pidHint < 4 && s_pointers[pidHint].active) return pidHint;
    return onlyIdx;
}

static void ReleasePointerSlot(int idx, int x, int y, int maskedAction) {
    if (idx < 0 || idx >= 4) return;
    SMobilePointerSlot& slot = s_pointers[idx];
    if (!slot.active) return;

    int rx = (x >= 0) ? x : slot.curX;
    int ry = (y >= 0) ? y : slot.curY;

    if (slot.role == TOUCH_ROLE_JOYSTICK) {
        ResetJoystickState();
    } else if (slot.role == TOUCH_ROLE_UI) {
        slot.active = false;
        slot.role = TOUCH_ROLE_NONE;
        slot.isDraggingCamera = false;
        UpdateUiTouchDownFlag();
        g_AndroidMouseX = rx;
        g_AndroidMouseY = ry;
        UI::CWindowManager::Instance().RunMouseMove(rx, ry);
        UI::CWindowManager::Instance().RunMouseLeftButtonUp(rx, ry);
        return;
    } else if (slot.role == TOUCH_ROLE_WORLD) {
        DWORD heldMs = ELTimer_GetMSec() - slot.downTimeMs;
        int totalDx = rx - slot.startX;
        int totalDy = ry - slot.startY;
        if (maskedAction != 3 && !slot.isDraggingCamera && heldMs < 250 && (totalDx * totalDx + totalDy * totalDy <= 196)) {
            g_AndroidMouseX = rx;
            g_AndroidMouseY = ry;
            UI::CWindowManager::Instance().RunMouseMove(rx, ry);
            UI::CWindowManager::Instance().RunMouseLeftButtonDown(rx, ry);
            UI::CWindowManager::Instance().RunMouseLeftButtonUp(rx, ry);
        }
    }
    slot.active = false;
    slot.role = TOUCH_ROLE_NONE;
    slot.isDraggingCamera = false;
    if (CountActiveWorldPointers() < 2)
        s_fLastPinchSpan = -1.0f;
    UpdateUiTouchDownFlag();
}

static bool IsPointOnUILayer(int x, int y) {
    UI::CWindowManager::Instance().RunMouseMove(x, y);
    UI::CWindow* pPointWnd = UI::CWindowManager::Instance().GetPointWindow();
    for (UI::CWindow* pCur = pPointWnd; pCur != nullptr; pCur = pCur->GetParent()) {
        const char* szName = pCur->GetName();
        if (szName && (strcmp(szName, "UI") == 0 || strcmp(szName, "UI_BOTTOM") == 0 || strcmp(szName, "TOP_MOST") == 0)) {
            return true;
        }
    }
    return false;
}

static PyObject* mobileInput_SetJoystickZone(PyObject* self, PyObject* args) {
    int cx = 148, cy = 562, r = 84;
    if (!PyArg_ParseTuple(args, "iii", &cx, &cy, &r))
        return Py_BuildValue("i", 0);
    s_iJoyCenterX = cx;
    s_iJoyCenterY = cy;
    s_iJoyRadius  = (r > 16) ? r : 84;
    return Py_BuildValue("i", 1);
}

static PyObject* mobileInput_GetJoystickState(PyObject* self, PyObject* args) {
    return Py_BuildValue("(iffff)", s_bJoyActive ? 1 : 0, s_fJoyNormX, s_fJoyNormY, s_fJoyRawDx, s_fJoyRawDy);
}

static PyObject* mobileInput_IsUiTouchDown(PyObject* self, PyObject* args) {
    return Py_BuildValue("i", s_bUiTouchDown ? 1 : 0);
}

static PyObject* mobileInput_IsRectTouchHeld(PyObject* self, PyObject* args) {
    int rx = 0, ry = 0, rw = 0, rh = 0;
    if (!PyArg_ParseTuple(args, "iiii", &rx, &ry, &rw, &rh))
        return Py_BuildValue("i", 0);
    for (int i = 0; i < 4; ++i) {
        if (!s_pointers[i].active || s_pointers[i].role == TOUCH_ROLE_JOYSTICK)
            continue;
        int sx = s_pointers[i].startX;
        int sy = s_pointers[i].startY;
        if (sx >= rx && sx < (rx + rw) && sy >= ry && sy < (ry + rh))
            return Py_BuildValue("i", 1);
    }
    return Py_BuildValue("i", 0);
}

static PyObject* mobileInput_GetScreenSize(PyObject* self, PyObject* args) {
    return Py_BuildValue("(ii)", g_AndroidScreenWidth, g_AndroidScreenHeight);
}

static PyObject* mobileInput_ShowKeyboard(PyObject* self, PyObject* args) {
    int show = 1;
    if (!PyArg_ParseTuple(args, "|i", &show))
        return Py_BuildValue("i", 0);
    AndroidToggleSoftKeyboard(show != 0);
    return Py_BuildValue("i", 1);
}

static PyObject* mobileInput_FindNearestEnemyVID(PyObject* self, PyObject* args) {
    float maxDist = 2500.0f;
    int cycleNext = 0;
    if (!PyArg_ParseTuple(args, "|fi", &maxDist, &cycleNext))
        return Py_BuildValue("i", 0);

    CPythonCharacterManager* pChrMgr = CPythonCharacterManager::InstancePtr();
    CInstanceBase* pMain = pChrMgr ? pChrMgr->GetMainInstancePtr() : nullptr;
    if (!pMain)
        return Py_BuildValue("i", 0);

    TPixelPosition kMainPos;
    pMain->NEW_GetPixelPosition(&kMainPos);

    struct SCand { DWORD vid; float distSq; };
    std::vector<SCand> cands;
    float maxDistSq = maxDist * maxDist;

    for (CPythonCharacterManager::CharacterIterator it = pChrMgr->CharacterInstanceBegin();
         it != pChrMgr->CharacterInstanceEnd(); ++it) {
        CInstanceBase* pkInst = *it;
        if (!pkInst || pkInst == pMain || pkInst->IsDead())
            continue;
        if (!pkInst->IsEnemy() && !pkInst->IsStone())
            continue;

        TPixelPosition kPos;
        pkInst->NEW_GetPixelPosition(&kPos);
        float dx = kPos.x - kMainPos.x;
        float dy = kPos.y - kMainPos.y;
        float dSq = dx * dx + dy * dy;
        if (dSq <= maxDistSq) {
            cands.push_back({ pkInst->GetVirtualID(), dSq });
        }
    }

    if (cands.empty()) {
        s_dwLastLockedVID = 0;
        return Py_BuildValue("i", 0);
    }

    std::sort(cands.begin(), cands.end(), [](const SCand& a, const SCand& b) {
        return a.distSq < b.distSq;
    });

    DWORD chosenVID = cands[0].vid;
    if (cycleNext && cands.size() > 1 && s_dwLastLockedVID != 0) {
        for (size_t i = 0; i < cands.size(); ++i) {
            if (cands[i].vid == s_dwLastLockedVID) {
                chosenVID = cands[(i + 1) % std::min<size_t>(cands.size(), 5)].vid;
                break;
            }
        }
    }
    s_dwLastLockedVID = chosenVID;
    if (CPythonPlayer::InstancePtr()) {
        CPythonPlayer::Instance().SetTarget(chosenVID, TRUE);
    }
    return Py_BuildValue("i", (int)chosenVID);
}

#define ENABLE_MOBILE_PERF_DEBUGGER // <-- Comment out to disable the profiler

#ifdef ENABLE_MOBILE_PERF_DEBUGGER
#include <time.h>

struct SMobileGLFrameStats {
    uint32_t drawCalls[8];
    uint32_t tris[8];
    uint64_t cpuTimeNs[8];
    uint32_t vboUploads;
    uint32_t vboUploadBytes;
    uint32_t texBinds;
    uint32_t progSwitches;
    uint32_t fboSwitches;
};
extern "C" SMobileGLFrameStats g_mobileGLStats;
__attribute__((weak)) float g_androidSubTimingMs[8] = {0.0f};

enum EMobilePhaseId {
    MPHASE_UPDATE_GAME   = 0,
    MPHASE_RENDER_GAME   = 1,
    MPHASE_TEXTTAIL_UPD  = 2,
    MPHASE_TEXTTAIL_ARR  = 3,
    MPHASE_TEXTTAIL_RND  = 4,
    MPHASE_CHR_PICK      = 5,
    MPHASE_COUNT         = 6
};

static double s_phaseAccumMs[MPHASE_COUNT] = {0};
static char s_perfHudLines[5][192] = {{0}};

static PyObject* mobileInput_RecordPhaseTime(PyObject* /*self*/, PyObject* args) {
    int phaseId = 0;
    float elapsedMs = 0.0f;
    if (PyArg_ParseTuple(args, "if", &phaseId, &elapsedMs)) {
        if (phaseId >= 0 && phaseId < MPHASE_COUNT)
            s_phaseAccumMs[phaseId] += (double)elapsedMs;
    }
    return Py_BuildValue("i", 1);
}

static PyObject* mobileInput_GetPerfHudText(PyObject* /*self*/, PyObject* /*args*/) {
    return Py_BuildValue("(sssss)",
        s_perfHudLines[0], s_perfHudLines[1], s_perfHudLines[2], s_perfHudLines[3], s_perfHudLines[4]);
}
#endif // ENABLE_MOBILE_PERF_DEBUGGER

static PyMethodDef s_mobileInputMethods[] = {
    { "SetJoystickZone",     mobileInput_SetJoystickZone,     METH_VARARGS, "Set virtual joystick zone" },
    { "GetJoystickState",    mobileInput_GetJoystickState,    METH_VARARGS, "Get joystick state tuple" },
    { "IsUiTouchDown",       mobileInput_IsUiTouchDown,       METH_VARARGS, "Check if UI touch is held" },
    { "IsRectTouchHeld",     mobileInput_IsRectTouchHeld,     METH_VARARGS, "Check if any touch started in rect" },
    { "GetScreenSize",       mobileInput_GetScreenSize,       METH_VARARGS, "Get native Android screen size" },
    { "ShowKeyboard",        mobileInput_ShowKeyboard,        METH_VARARGS, "Show or hide Android soft keyboard" },
    { "FindNearestEnemyVID", mobileInput_FindNearestEnemyVID, METH_VARARGS, "Find nearest enemy VID" },
#ifdef ENABLE_MOBILE_PERF_DEBUGGER
    { "RecordPhaseTime",     mobileInput_RecordPhaseTime,     METH_VARARGS, "Record engine phase ms" },
    { "GetPerfHudText",      mobileInput_GetPerfHudText,      METH_VARARGS, "Get 5-line perf HUD strings" },
#endif
    { NULL, NULL, 0, NULL }
};

void initmobileInput(void) {
    Py_InitModule("mobileInput", s_mobileInputMethods);
}
#endif // ENABLE_OPENGL

JNIEXPORT void JNICALL Java_com_metin2_client_NativeLib_render(JNIEnv* env, jobject obj) {
    if (s_bEngineReady && CPythonApplication::InstancePtr() && CStateManager::InstancePtr()) {
        while (true) {
            pthread_mutex_lock(&s_inputQueueMutex);
            if (s_touchTail == s_touchHead) {
                pthread_mutex_unlock(&s_inputQueueMutex);
                break;
            }
            AndroidTouchEvt ev = s_touchQueue[s_touchTail];
            s_touchTail = (s_touchTail + 1) & 255;
            pthread_mutex_unlock(&s_inputQueueMutex);

            static bool s_bMouseModuleChecked = false;
            if (!s_bMouseModuleChecked) {
                s_bMouseModuleChecked = true;
                PyRun_SimpleString(
                    "import sys\n"
                    "_mm = sys.modules.get('mouseModule')\n"
                    "if _mm is not None and getattr(_mm, 'mouseController', None) is None:\n"
                    "    _mm.mouseController = _mm.CMouseController()\n"
                    "    _mm.mouseController.Create()\n"
                );
            }

#ifdef ENABLE_OPENGL
            int pid = (ev.pointerId >= 0 && ev.pointerId < 4) ? ev.pointerId : 0;
            int maskedAction = ev.action & 0xFF;

            if (maskedAction == 3) { // ACTION_CANCEL: release all slots and joystick immediately
                for (int i = 0; i < 4; ++i) {
                    if (s_pointers[i].active)
                        ReleasePointerSlot(i, ev.x, ev.y, 3);
                }
                s_bUiTouchDown = false;
                s_fLastPinchSpan = -1.0f;
                ResetJoystickState();
                continue;
            }

            if (maskedAction == 0 || maskedAction == 5) { // ACTION_DOWN or ACTION_POINTER_DOWN
                if (maskedAction == 0) {
                    for (int i = 0; i < 4; ++i) {
                        s_pointers[i].active = false;
                        s_pointers[i].role = TOUCH_ROLE_NONE;
                        s_pointers[i].isDraggingCamera = false;
                    }
                    s_bUiTouchDown = false;
                    s_fLastPinchSpan = -1.0f;
                    if (s_bJoyActive) ResetJoystickState();
                } else if (s_pointers[pid].active) {
                    for (int i = 0; i < 4; ++i) {
                        if (!s_pointers[i].active) {
                            pid = i;
                            break;
                        }
                    }
                }

                SMobilePointerSlot& slot = s_pointers[pid];
                slot.active = true;
                slot.startX = slot.curX = slot.lastX = ev.x;
                slot.startY = slot.curY = slot.lastY = ev.y;
                slot.downTimeMs = ELTimer_GetMSec();
                slot.isDraggingCamera = false;

                int jdx = ev.x - s_iJoyCenterX;
                int jdy = ev.y - s_iJoyCenterY;
                int joyCatchRadius = s_iJoyRadius + 24;
                bool bInGameWorld = (CPythonCharacterManager::InstancePtr() && CPythonCharacterManager::Instance().GetMainInstancePtr() != nullptr);

                // 1. Virtual Joystick (Bottom-Left)
                if (bInGameWorld && !s_bJoyActive && (jdx * jdx + jdy * jdy <= joyCatchRadius * joyCatchRadius)) {
                    slot.role = TOUCH_ROLE_JOYSTICK;
                    s_bJoyActive = true;
                    UpdateJoystickFromPoint(ev.x, ev.y);
                }
                // 2. Interactive UI Window / HUD Button
                else if (IsPointOnUILayer(ev.x, ev.y)) {
                    slot.role = TOUCH_ROLE_UI;
                    UpdateUiTouchDownFlag();
                    g_AndroidMouseX = ev.x;
                    g_AndroidMouseY = ev.y;
                    UI::CWindowManager::Instance().RunMouseMove(ev.x, ev.y);
                    UI::CWindowManager::Instance().RunMouseLeftButtonDown(ev.x, ev.y);
                }
                // 3. 3D World (Orbit / Pinch-to-Zoom / Fast-Tap)
                else {
                    slot.role = TOUCH_ROLE_WORLD;
                    // Stop existing coasting inertia when placing a finger on the world
                    s_fCamVelRoll *= 0.25f;
                    s_fCamVelPitch *= 0.25f;
                    if (!s_bUiTouchDown) {
                        g_AndroidMouseX = ev.x;
                        g_AndroidMouseY = ev.y;
                        UI::CWindowManager::Instance().RunMouseMove(ev.x, ev.y);
                    }
                }
            }
            else if (maskedAction == 2) { // ACTION_MOVE
                int targetSlot = FindBestActiveSlotForPoint(pid, ev.x, ev.y, true);
                if (targetSlot < 0) {
                    targetSlot = pid;
                    SMobilePointerSlot& newSlot = s_pointers[targetSlot];
                    newSlot.active = true;
                    newSlot.startX = newSlot.curX = newSlot.lastX = ev.x;
                    newSlot.startY = newSlot.curY = newSlot.lastY = ev.y;
                    newSlot.downTimeMs = ELTimer_GetMSec();
                    newSlot.isDraggingCamera = false;
                    int jdx = ev.x - s_iJoyCenterX;
                    int jdy = ev.y - s_iJoyCenterY;
                    int joyCatchRadius = s_iJoyRadius + 24;
                    if (!s_bJoyActive && (jdx * jdx + jdy * jdy <= joyCatchRadius * joyCatchRadius)) {
                        newSlot.role = TOUCH_ROLE_JOYSTICK;
                        s_bJoyActive = true;
                    } else {
                        newSlot.role = TOUCH_ROLE_WORLD;
                    }
                }

                SMobilePointerSlot& slot = s_pointers[targetSlot];
                int stepDx = ev.x - slot.lastX;
                int stepDy = ev.y - slot.lastY;
                slot.curX = slot.lastX = ev.x;
                slot.curY = slot.lastY = ev.y;

                if (slot.role == TOUCH_ROLE_JOYSTICK) {
                    UpdateJoystickFromPoint(ev.x, ev.y);
                }
                else if (slot.role == TOUCH_ROLE_UI) {
                    g_AndroidMouseX = ev.x;
                    g_AndroidMouseY = ev.y;
                    UI::CWindowManager::Instance().RunMouseMove(ev.x, ev.y);
                }
                else if (slot.role == TOUCH_ROLE_WORLD) {
                    int wIdx0 = -1, wIdx1 = -1;
                    int worldTouchCount = CountActiveWorldPointers(&wIdx0, &wIdx1);
                    if (worldTouchCount >= 2 && wIdx0 >= 0 && wIdx1 >= 0) {
                        // Two-finger Pinch-to-Zoom on the 3D World
                        s_pointers[wIdx0].isDraggingCamera = true;
                        s_pointers[wIdx1].isDraggingCamera = true;
                        float pdx = (float)(s_pointers[wIdx0].curX - s_pointers[wIdx1].curX);
                        float pdy = (float)(s_pointers[wIdx0].curY - s_pointers[wIdx1].curY);
                        float span = hypotf(pdx, pdy);
                        if (s_fLastPinchSpan > 15.0f && span > 15.0f) {
                            float deltaSpan = span - s_fLastPinchSpan;
                            if (CCamera* pCam = CCameraManager::Instance().GetCurrentCamera()) {
                                if (s_fBaseCamDistance < 200.0f)
                                    s_fBaseCamDistance = pCam->GetDistance();
                                s_fBaseCamDistance -= deltaSpan * 3.2f;
                                if (s_fBaseCamDistance < 250.0f) s_fBaseCamDistance = 250.0f;
                                if (s_fBaseCamDistance > 2500.0f) s_fBaseCamDistance = 2500.0f;
                            }
                        }
                        s_fLastPinchSpan = span;
                        s_fManualCamCooldown = 1.25f;
                    } else {
                        s_fLastPinchSpan = -1.0f;
                        int totalDx = ev.x - slot.startX;
                        int totalDy = ev.y - slot.startY;
                        if (!slot.isDraggingCamera && (totalDx * totalDx + totalDy * totalDy > 49)) {
                            slot.isDraggingCamera = true;
                        }
                        if (slot.isDraggingCamera) {
                            // Accumulate sub-pixel deltas for cinematic frame-interpolated orbit + inertia
                            s_fCamAccumDx += (float)stepDx;
                            s_fCamAccumDy += (float)stepDy;
                            s_fManualCamCooldown = 1.25f;
                        }
                    }
                }
            }
            else if (maskedAction == 6) { // ACTION_POINTER_UP
                int targetSlot = FindBestActiveSlotForPoint(pid, ev.x, ev.y, false);
                if (targetSlot >= 0) {
                    ReleasePointerSlot(targetSlot, ev.x, ev.y, 6);
                }
            }
            else if (maskedAction == 1) { // ACTION_UP
                int primarySlot = FindBestActiveSlotForPoint(pid, ev.x, ev.y, false);
                if (primarySlot >= 0) {
                    ReleasePointerSlot(primarySlot, ev.x, ev.y, 1);
                }
                for (int i = 0; i < 4; ++i) {
                    if (s_pointers[i].active) {
                        ReleasePointerSlot(i, s_pointers[i].curX, s_pointers[i].curY, 1);
                    }
                }
            }
#endif
        }

#ifdef ENABLE_OPENGL
        // ====================================================================
        // Cinematic Camera Update + 360-Degree Joystick Movement
        // ====================================================================
        static DWORD s_dwLastJoyFrameMs = ELTimer_GetMSec();
        DWORD dwNowMs = ELTimer_GetMSec();
        float fDt = (dwNowMs > s_dwLastJoyFrameMs) ? ((float)(dwNowMs - s_dwLastJoyFrameMs) * 0.001f) : (1.0f / 60.0f);
        s_dwLastJoyFrameMs = dwNowMs;
        if (fDt > 0.1f) fDt = 0.1f;
        if (fDt < 0.001f) fDt = 0.001f;

        if (s_fManualCamCooldown > 0.0f) {
            s_fManualCamCooldown = fmaxf(0.0f, s_fManualCamCooldown - fDt);
        }

        CCamera* pCam = CCameraManager::InstancePtr() ? CCameraManager::Instance().GetCurrentCamera() : nullptr;

        // 1. Silky-Smooth Damped Touch Orbit & Post-Release Glide Inertia
        if (pCam) {
            const bool isDragging = IsAnyPointerDraggingCamera() && (CountActiveWorldPointers() == 1);
            if (isDragging) {
                float targetVelRoll  = (s_fCamAccumDx * 0.21f) / fDt;
                float targetVelPitch = (s_fCamAccumDy * 0.15f) / fDt;
                s_fCamAccumDx = 0.0f;
                s_fCamAccumDy = 0.0f;

                // Clamp peak angular velocity to prevent wild snaps on touch spikes
                targetVelRoll  = fmaxf(-540.0f, fminf(540.0f, targetVelRoll));
                targetVelPitch = fmaxf(-360.0f, fminf(360.0f, targetVelPitch));

                // Exponential low-pass filter removes touch digitizer stair-stepping
                float alpha = 1.0f - expf(-26.0f * fDt);
                s_fCamVelRoll  += (targetVelRoll  - s_fCamVelRoll)  * alpha;
                s_fCamVelPitch += (targetVelPitch - s_fCamVelPitch) * alpha;
            } else {
                s_fCamAccumDx = 0.0f;
                s_fCamAccumDy = 0.0f;
                // Natural cinematic flick coasting decay
                float decay = expf(-8.5f * fDt);
                s_fCamVelRoll  *= decay;
                s_fCamVelPitch *= decay;
                if (fabsf(s_fCamVelRoll)  < 0.5f) s_fCamVelRoll  = 0.0f;
                if (fabsf(s_fCamVelPitch) < 0.5f) s_fCamVelPitch = 0.0f;
            }

            if (s_fCamVelRoll != 0.0f || s_fCamVelPitch != 0.0f) {
                float pitchStep = s_fCamVelPitch * fDt;
                float rollStep  = s_fCamVelRoll  * fDt;
                if (pCam->GetCameraState() == CAMERA_STATE_CANTGODOWN && pitchStep < 0.0f) {
                    pitchStep = 0.0f;
                    s_fCamVelPitch = 0.0f;
                }
                pCam->RotateEyeAroundTarget(pitchStep, rollStep);
            }
        }

        // 2. Verify Joystick Ownership & Drive Character + Delayed Camera Follow-Up
        if (s_bJoyActive) {
            bool hasJoyOwner = false;
            for (int i = 0; i < 4; ++i) {
                if (s_pointers[i].active && s_pointers[i].role == TOUCH_ROLE_JOYSTICK) {
                    hasJoyOwner = true;
                    break;
                }
            }
            if (!hasJoyOwner) {
                ResetJoystickState();
            }
        }

        static float s_fJoySteerHoldTime = 0.0f;
        float targetAutoRollRate = 0.0f;

        if (s_bJoyActive && CPythonPlayer::InstancePtr() && CPythonCharacterManager::InstancePtr() &&
            CPythonCharacterManager::Instance().GetMainInstancePtr() != nullptr) {
            CAndroidPlayerAccessor& rkPlayer = static_cast<CAndroidPlayerAccessor&>(CPythonPlayer::Instance());
            CInstanceBase* pMain = CPythonCharacterManager::Instance().GetMainInstancePtr();
            float mag = hypotf(s_fJoyNormX, s_fJoyNormY);
            if (mag > 0.15f) {
                rkPlayer.NEW_SetAutoCameraRotationSpeed(0.0f);
                s_fJoySteerHoldTime += fDt;

                // Delay rotational follow by 0.35s so character turns & moves first, then camera smoothly follows up
                if (!IsAnyPointerDraggingCamera() && s_fManualCamCooldown <= 0.0f && pCam) {
                    float delayWeight = SmoothStepF(0.35f, 1.05f, s_fJoySteerHoldTime);
                    float curveX = s_fJoyNormX * (0.40f + 0.60f * fabsf(s_fJoyNormX));
                    targetAutoRollRate = curveX * 48.0f * delayWeight;
                }

                float vx = s_fJoyNormX / mag;
                float vy = s_fJoyNormY / mag;
                float dot = -vy;
                if (dot < -1.0f) dot = -1.0f;
                else if (dot > 1.0f) dot = 1.0f;
                float ret = acosf(dot) * (180.0f / 3.14159265f);
                if (vx < 0.0f) ret = 360.0f - ret;
                float fDeg = 360.0f - ret;

                if (!s_bJoyMovingPlayer) {
                    rkPlayer.__ClearAutoAttackTargetActorID();
                }
                if (!pMain->IsUsingSkill()) {
                    rkPlayer.NEW_MoveToDirection(fDeg);
                }
                s_bJoyMovingPlayer = true;
            } else if (s_bJoyMovingPlayer) {
                if (!pMain->IsAttacking() && !pMain->IsUsingSkill()) {
                    rkPlayer.NEW_Stop();
                }
                s_bJoyMovingPlayer = false;
                s_fJoySteerHoldTime = 0.0f;
            }
        } else {
            s_fJoySteerHoldTime = 0.0f;
        }

        // Apply delayed rotational follow-up & smooth coasting after stick release
        if (pCam && !IsAnyPointerDraggingCamera() && s_fManualCamCooldown <= 0.0f) {
            float rateBlend = (fabsf(targetAutoRollRate) > fabsf(s_fSmoothedAutoRollRate))
                ? (1.0f - expf(-3.2f * fDt))   // Gentle, delayed build-up when starting to turn
                : (1.0f - expf(-4.5f * fDt));  // Smooth coasting settle after releasing stick
            s_fSmoothedAutoRollRate += (targetAutoRollRate - s_fSmoothedAutoRollRate) * rateBlend;
            if (fabsf(s_fSmoothedAutoRollRate) > 0.05f) {
                pCam->RotateEyeAroundTarget(0.0f, s_fSmoothedAutoRollRate * fDt);
            } else {
                s_fSmoothedAutoRollRate = 0.0f;
            }
        } else {
            s_fSmoothedAutoRollRate *= expf(-10.0f * fDt);
        }

        // 3. Apply Smooth Two-Finger Pinch Zoom Interpolation
        if (pCam && !CPythonApplication::Instance().IsFirstPersonMode()) {
            float curCamDist = pCam->GetDistance();
            if (s_fBaseCamDistance < 200.0f) {
                s_fBaseCamDistance = curCamDist;
            } else if (fabsf(curCamDist - s_fBaseCamDistance) > 180.0f && CountActiveWorldPointers() < 2) {
                s_fBaseCamDistance = curCamDist;
            }

            float targetDist = fmaxf(200.0f, fminf(2500.0f, s_fBaseCamDistance));
            float newDist = curCamDist + (targetDist - curCamDist) * (1.0f - expf(-10.0f * fDt));
            if (fabsf(newDist - curCamDist) > 0.05f) {
                pCam->SetDistance(newDist);
            }
        }
#endif

		while (true) {
            pthread_mutex_lock(&s_inputQueueMutex);
            if (s_keyTail == s_keyHead) {
                pthread_mutex_unlock(&s_inputQueueMutex);
                break;
            }
            AndroidKeyEvt kev = s_keyQueue[s_keyTail];
            s_keyTail = (s_keyTail + 1) & 63;
            pthread_mutex_unlock(&s_inputQueueMutex);
            if (kev.keyCode == 67 || kev.unicodeChar == 8) {
                PyRun_SimpleString("import ime\nime.PasteBackspace()\n");
                UI::CWindowManager::Instance().RunIMEUpdate();
            } else if (kev.keyCode == 66 || kev.keyCode == 160 || kev.unicodeChar == 10 || kev.unicodeChar == 13) {
                UI::CWindow* pBefore = UI::CWindowManager::Instance().GetActivateWindow();
                UI::CWindowManager::Instance().RunIMEReturnEvent();
                UI::CWindow* pAfter = UI::CWindowManager::Instance().GetActivateWindow();
                if (!pAfter || pAfter == pBefore) {
                    AndroidToggleSoftKeyboard(false);
                }
            } else if (kev.keyCode == 61) {
                UI::CWindowManager::Instance().RunIMETabEvent();
            } else if (kev.unicodeChar >= 32) {
                unsigned char outCh = 0;
                if (kev.unicodeChar < 127) {
                    outCh = (unsigned char)kev.unicodeChar;
                } else {
                    static const uint16_t s_cp1256Map[128] = {
                        0x20AC,0x067E,0x201A,0x0192,0x201E,0x2026,0x2020,0x2021,0x02C6,0x2030,0x0679,0x2039,0x0152,0x0686,0x0698,0x0688,
                        0x06AF,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,0x06A9,0x2122,0x0691,0x203A,0x0153,0x200C,0x200D,0x06BA,
                        0x00A0,0x060C,0x00A2,0x00A3,0x00A4,0x00A5,0x00A6,0x00A7,0x00A8,0x00A9,0x06BE,0x00AB,0x00AC,0x00AD,0x00AE,0x00AF,
                        0x00B0,0x00B1,0x00B2,0x00B3,0x00B4,0x00B5,0x00B6,0x00B7,0x00B8,0x00B9,0x061B,0x00BB,0x00BC,0x00BD,0x00BE,0x061F,
                        0x06C1,0x0621,0x0622,0x0623,0x0624,0x0625,0x0626,0x0627,0x0628,0x0629,0x062A,0x062B,0x062C,0x062D,0x062E,0x062F,
                        0x0630,0x0631,0x0632,0x0633,0x0634,0x0635,0x0636,0x00D7,0x0637,0x0638,0x0639,0x063A,0x0640,0x0641,0x0642,0x0643,
                        0x00E0,0x0644,0x00E2,0x0645,0x0646,0x0647,0x0648,0x00E7,0x00E8,0x00E9,0x00EA,0x00EB,0x0649,0x064A,0x00EE,0x00EF,
                        0x064B,0x064C,0x064D,0x064E,0x00F4,0x064F,0x0650,0x00F7,0x0651,0x00F9,0x0652,0x00FB,0x00FC,0x200E,0x200F,0x06D2
                    };
                    for (int idx = 0; idx < 128; ++idx) {
                        if (s_cp1256Map[idx] == (uint16_t)kev.unicodeChar) {
                            outCh = (unsigned char)(0x80 + idx);
                            break;
                        }
                    }
                }
                if (outCh != 0) {
                    char cmd[64];
                    snprintf(cmd, sizeof(cmd), "import ime\nime.PasteString('\\x%02x')\n", (unsigned int)outCh);
                    PyRun_SimpleString(cmd);
                    UI::CWindowManager::Instance().RunIMEUpdate();
                }
            }
        }
        #ifdef ENABLE_MOBILE_PERF_DEBUGGER
        const bool bInGameWorldNow = (CPythonCharacterManager::InstancePtr() &&
                                      CPythonCharacterManager::Instance().GetMainInstancePtr() != nullptr);
        if (bInGameWorldNow) {
            static bool s_bPyPerfHooked = false;
            if (!s_bPyPerfHooked) {
                s_bPyPerfHooked = true;
                PyRun_SimpleString(
                    "try:\n"
                    "    import app, chr, textTail, ui, mobileInput, time as _t\n"
                    "    def _wrap_phase(mod, name, pid):\n"
                    "        orig = getattr(mod, name, None)\n"
                    "        if orig is None or getattr(orig, '_prof_wrapped', False): return\n"
                    "        def _wrapped(*a, **k):\n"
                    "            t0 = _t.time()\n"
                    "            r = orig(*a, **k)\n"
                    "            mobileInput.RecordPhaseTime(pid, (_t.time() - t0) * 1000.0)\n"
                    "            return r\n"
                    "        _wrapped._prof_wrapped = True\n"
                    "        setattr(mod, name, _wrapped)\n"
                    "    _wrap_phase(app, 'UpdateGame', 0)\n"
                    "    _wrap_phase(app, 'RenderGame', 1)\n"
                    "    _wrap_phase(textTail, 'UpdateAllTextTail', 2)\n"
                    "    _wrap_phase(textTail, 'UpdateShowingTextTail', 2)\n"
                    "    _wrap_phase(textTail, 'ArrangeTextTail', 3)\n"
                    "    _wrap_phase(textTail, 'Render', 4)\n"
                    "    _wrap_phase(chr, 'Pick', 5)\n"
                    "    class _MobilePerfHud(ui.Window):\n"
                    "        def __init__(self):\n"
                    "            ui.Window.__init__(self, 'TOP_MOST')\n"
                    "            self.SetSize(720, 95)\n"
                    "            self.SetPosition(155, 4)\n"
                    "            self.AddFlag('not_pick')\n"
                    "            self.lines = []\n"
                    "            for i in xrange(5):\n"
                    "                tl = ui.TextLine()\n"
                    "                tl.SetParent(self)\n"
                    "                tl.SetPosition(0, i * 16)\n"
                    "                tl.SetOutline()\n"
                    "                tl.SetPackedFontColor(0xFF00FF66 if i == 0 else (0xFFFFDD66 if i < 3 else 0xFF88EEFF))\n"
                    "                tl.Show()\n"
                    "                self.lines.append(tl)\n"
                    "            self.lastUpd = 0.0\n"
                    "            self.Show()\n"
                    "        def OnUpdate(self):\n"
                    "            now = _t.time()\n"
                    "            if now - self.lastUpd >= 0.5:\n"
                    "                self.lastUpd = now\n"
                    "                txts = mobileInput.GetPerfHudText()\n"
                    "                for i in xrange(5):\n"
                    "                    self.lines[i].SetText(txts[i])\n"
                    "    _g_mobilePerfHud = _MobilePerfHud()\n"
                    "except:\n"
                    "    pass\n"
                );
            }
        }

        struct timespec tsLoop0, tsLoop1;
        clock_gettime(CLOCK_MONOTONIC, &tsLoop0);
        static uint64_t s_lastFrameEndNs = 0;
        uint64_t loopStartNs = (uint64_t)tsLoop0.tv_sec * 1000000000ULL + (uint64_t)tsLoop0.tv_nsec;

        CPythonApplication::InstancePtr()->Loop();

        clock_gettime(CLOCK_MONOTONIC, &tsLoop1);
        uint64_t loopEndNs = (uint64_t)tsLoop1.tv_sec * 1000000000ULL + (uint64_t)tsLoop1.tv_nsec;

        static uint32_t s_profFrames = 0;
        static double s_accumLoopMs = 0.0;
        static double s_accumSwapMs = 0.0;
        static SMobileGLFrameStats s_accumGL = {};
        static DWORD s_lastReportMs = ELTimer_GetMSec();

        s_profFrames++;
        s_accumLoopMs += (double)(loopEndNs - loopStartNs) * 1e-6;
        if (s_lastFrameEndNs > 0 && loopStartNs > s_lastFrameEndNs)
            s_accumSwapMs += (double)(loopStartNs - s_lastFrameEndNs) * 1e-6;
        s_lastFrameEndNs = loopEndNs;

        for (int b = 0; b < 8; ++b) {
            s_accumGL.drawCalls[b] += g_mobileGLStats.drawCalls[b];
            s_accumGL.tris[b]      += g_mobileGLStats.tris[b];
            s_accumGL.cpuTimeNs[b] += g_mobileGLStats.cpuTimeNs[b];
        }
        s_accumGL.vboUploads     += g_mobileGLStats.vboUploads;
        s_accumGL.vboUploadBytes += g_mobileGLStats.vboUploadBytes;
        s_accumGL.texBinds       += g_mobileGLStats.texBinds;
        s_accumGL.progSwitches   += g_mobileGLStats.progSwitches;
        s_accumGL.fboSwitches    += g_mobileGLStats.fboSwitches;
        memset(&g_mobileGLStats, 0, sizeof(g_mobileGLStats));

        DWORD nowReportMs = ELTimer_GetMSec();
        if (nowReportMs - s_lastReportMs >= 1500 && s_profFrames > 0) {
            double invF = 1.0 / (double)s_profFrames;
            double elapsedSec = (double)(nowReportMs - s_lastReportMs) * 0.001;
            double realFps = (double)s_profFrames / (elapsedSec > 0.001 ? elapsedSec : 1.0);
            double avgLoopMs = s_accumLoopMs * invF;
            double avgSwapMs = s_accumSwapMs * invF;
            double totalFrameMs = (avgLoopMs + avgSwapMs) > 0.01 ? (avgLoopMs + avgSwapMs) : 1.0;

            double phUpd  = s_phaseAccumMs[MPHASE_UPDATE_GAME]  * invF;
            double phRnd  = s_phaseAccumMs[MPHASE_RENDER_GAME]  * invF;
            double phTTU  = s_phaseAccumMs[MPHASE_TEXTTAIL_UPD] * invF;
            double phTTA  = s_phaseAccumMs[MPHASE_TEXTTAIL_ARR] * invF;
            double phTTR  = s_phaseAccumMs[MPHASE_TEXTTAIL_RND] * invF;
            double phPick = s_phaseAccumMs[MPHASE_CHR_PICK]     * invF;

            uint32_t totalDC = 0, totalTris = 0;
            double bMs[8];
            uint32_t bDc[8];
            for (int b = 0; b < 8; ++b) {
                bDc[b]    = (uint32_t)(s_accumGL.drawCalls[b] * invF);
                totalDC   += bDc[b];
                totalTris += (uint32_t)(s_accumGL.tris[b] * invF);
                bMs[b]    = (double)s_accumGL.cpuTimeNs[b] * 1e-6 * invF;
            }

            double gl3dSumMs = bMs[0] + bMs[1] + bMs[2] + bMs[3] + bMs[4] + bMs[5] + bMs[6];
            double cpuDeformAndCullMs = (phRnd > gl3dSumMs) ? (phRnd - gl3dSumMs) : 0.0;

            double subBGUpd  = g_androidSubTimingMs[0] * invF;
            double subIkaUpd = g_androidSubTimingMs[1] * invF;
            double subChrUpd = g_androidSubTimingMs[2] * invF;
            double subEftUpd = g_androidSubTimingMs[3] * invF;
            double subDeform = g_androidSubTimingMs[4] * invF;
            memset(g_androidSubTimingMs, 0, sizeof(g_androidSubTimingMs));

            const bool hasSubTimings = (subBGUpd + subIkaUpd + subChrUpd + subEftUpd) > 0.05;

            struct SSubsysCost { const char* name; double ms; uint32_t dc; };
            SSubsysCost costs[14] = {
                { "CPU Deform/Frustum/Sort", cpuDeformAndCullMs, 0 },
                { "Update: Characters/NPCs", hasSubTimings ? subChrUpd : phUpd, 0 },
                { "Update: Effects/Particles", subEftUpd, 0 },
                { "Update: OfflineShops/Evt",  subIkaUpd, 0 },
                { "Update: Map/Background",    subBGUpd,  0 },
                { "ShadowMap Depth Pass",    bMs[0],             bDc[0] },
                { "Water/Puddle Refl FBO",   bMs[1],             bDc[1] },
                { "Terrain Splat Shaders",   bMs[2],             bDc[2] },
                { "Map Objects & Trees",     bMs[3],             bDc[3] },
                { "Water Surface Shader",    bMs[4],             bDc[4] },
                { "Actors .gr2 Meshes GL",   bMs[5],             bDc[5] },
                { "3D Particles/Effects GL", bMs[6],             bDc[6] },
                { "TextTail Arrange+Upd",    phTTU + phTTA,      0 },
                { "2D UI Windows & Icons",   bMs[7],             bDc[7] }
            };
            std::sort(std::begin(costs), std::end(costs), [](const SSubsysCost& a, const SSubsysCost& b) {
                return a.ms > b.ms;
            });

            snprintf(s_perfHudLines[0], sizeof(s_perfHudLines[0]),
                "FPS: %.1f | Frame: %.1fms (CPU: %.1fms | GPU/Swap: %.1fms) | VBOUp: %u (%uKB)",
                realFps, avgLoopMs + avgSwapMs, avgLoopMs, avgSwapMs,
                (uint32_t)(s_accumGL.vboUploads * invF),
                (uint32_t)(s_accumGL.vboUploadBytes * invF / 1024));
            if (hasSubTimings) {
                snprintf(s_perfHudLines[1], sizeof(s_perfHudLines[1]),
                    "Rnd3D %.1fms (Def:%.1f GL:%.1f) | Upd %.1fms (Chr:%.1f Eft:%.1f Ika:%.1f BG:%.1f) | TT:%.1fms",
                    phRnd, subDeform > 0.0f ? subDeform : cpuDeformAndCullMs, gl3dSumMs,
                    phUpd, subChrUpd, subEftUpd, subIkaUpd, subBGUpd, phTTU + phTTA + phTTR);
            } else {
                snprintf(s_perfHudLines[1], sizeof(s_perfHudLines[1]),
                    "Phases: Render3D %.1fms (Deform/CPU:%.1fms GL:%.1fms) | Update %.1fms | TextTail %.1fms | Pick %.1fms",
                    phRnd, cpuDeformAndCullMs, gl3dSumMs, phUpd, phTTU + phTTA + phTTR, phPick);
            }
            snprintf(s_perfHudLines[2], sizeof(s_perfHudLines[2]),
                "GL: %u dc (%uk tri) | Act:%.1fms(%u) Shd:%.1fms(%u) Obj:%.1fms(%u) Ter:%.1fms(%u) FX:%.1fms(%u) UI:%.1fms(%u)",
                totalDC, totalTris / 1000,
                bMs[5], bDc[5], bMs[0], bDc[0], bMs[3], bDc[3], bMs[2], bDc[2], bMs[6], bDc[6], bMs[7], bDc[7]);
            snprintf(s_perfHudLines[3], sizeof(s_perfHudLines[3]),
                "Hot #1: %s = %.1fms (%.0f%%) [%u dc]",
                costs[0].name, costs[0].ms, (100.0 * costs[0].ms) / totalFrameMs, costs[0].dc);
            snprintf(s_perfHudLines[4], sizeof(s_perfHudLines[4]),
                "Hot #2: %s = %.1fms (%.0f%%) | #3: %s = %.1fms (%.0f%%)",
                costs[1].name, costs[1].ms, (100.0 * costs[1].ms) / totalFrameMs,
                costs[2].name, costs[2].ms, (100.0 * costs[2].ms) / totalFrameMs);

            LOGI("[PERF] %s", s_perfHudLines[0]);
            LOGI("[PERF] %s", s_perfHudLines[1]);
            LOGI("[PERF] %s", s_perfHudLines[2]);
            LOGI("[PERF] %s", s_perfHudLines[3]);
            LOGI("[PERF] %s", s_perfHudLines[4]);

            if (bInGameWorldNow) {
                if (FILE* pfp = fopen("perf_mobile.txt", "a")) {
                    fprintf(pfp, "=== PERF REPORT (%.1f FPS) ===\n%s\n%s\n%s\n%s\n%s\n",
                        realFps, s_perfHudLines[0], s_perfHudLines[1], s_perfHudLines[2], s_perfHudLines[3], s_perfHudLines[4]);
                    fclose(pfp);
                }
            }

            s_profFrames = 0;
            s_accumLoopMs = 0.0;
            s_accumSwapMs = 0.0;
            memset(&s_accumGL, 0, sizeof(s_accumGL));
            for (int i = 0; i < MPHASE_COUNT; ++i) s_phaseAccumMs[i] = 0.0;
            s_lastReportMs = nowReportMs;
        }
#else
        CPythonApplication::InstancePtr()->Loop();
#endif
    }
}

static inline int MapPhysXToLogical(float x) {
    if (g_AndroidPhysWidth <= 0) return (int)x;
    return (int)(x * (float)g_AndroidScreenWidth / (float)g_AndroidPhysWidth + 0.5f);
}

static inline int MapPhysYToLogical(float y) {
    if (g_AndroidPhysHeight <= 0) return (int)y;
    return (int)(y * (float)g_AndroidScreenHeight / (float)g_AndroidPhysHeight + 0.5f);
}

JNIEXPORT void JNICALL Java_com_metin2_client_NativeLib_touchEvent(JNIEnv* env, jobject obj, jint action, jfloat x, jfloat y) {
    int pid = (action >> 8) & 0x3;
    int masked = (int)(action & 0xFF);
    int lx = MapPhysXToLogical(x);
    int ly = MapPhysYToLogical(y);
    pthread_mutex_lock(&s_inputQueueMutex);
    int next = (s_touchHead + 1) & 255;
    if (next != s_touchTail) {
        s_touchQueue[s_touchHead] = { pid, masked, lx, ly };
        s_touchHead = next;
    } else if (masked == 1 || masked == 6 || masked == 3) {
        int prev = (s_touchHead - 1) & 255;
        s_touchQueue[prev] = { pid, masked, lx, ly };
    }
    pthread_mutex_unlock(&s_inputQueueMutex);
}

JNIEXPORT void JNICALL Java_com_metin2_client_NativeLib_multiTouchEvent(JNIEnv* env, jobject obj, jint pointerId, jint action, jfloat x, jfloat y) {
    int pid = (int)(pointerId & 3);
    int masked = (int)(action & 0xFF);
    int lx = MapPhysXToLogical(x);
    int ly = MapPhysYToLogical(y);
    pthread_mutex_lock(&s_inputQueueMutex);
    int next = (s_touchHead + 1) & 255;
    if (next != s_touchTail) {
        s_touchQueue[s_touchHead] = { pid, masked, lx, ly };
        s_touchHead = next;
    } else if (masked == 1 || masked == 6 || masked == 3) {
        int prev = (s_touchHead - 1) & 255;
        s_touchQueue[prev] = { pid, masked, lx, ly };
    }
    pthread_mutex_unlock(&s_inputQueueMutex);
}

JNIEXPORT void JNICALL Java_com_metin2_client_NativeLib_keyEvent(JNIEnv* env, jobject obj, jint keyCode, jint unicodeChar) {
    pthread_mutex_lock(&s_inputQueueMutex);
    int next = (s_keyHead + 1) & 63;
    if (next != s_keyTail) {
        s_keyQueue[s_keyHead] = { (int)keyCode, (int)unicodeChar };
        s_keyHead = next;
    }
    pthread_mutex_unlock(&s_inputQueueMutex);
}

// Win32 WebBrowser Stubs
int WebBrowser_Startup(HINSTANCE hInstance) { return 1; }
void WebBrowser_Cleanup() {}
void WebBrowser_Destroy() {}
int WebBrowser_Show(HWND hParent, const char* addr, const RECT* rcWebBrowser) { return 0; }
void WebBrowser_Hide() {}
void WebBrowser_Move(const RECT* rcWebBrowser) {}
int WebBrowser_IsVisible() { return 0; }
const RECT& WebBrowser_GetRect() { static RECT r = {0, 0, 0, 0}; return r; }

// DevIL Image Library Stubs
void ILAPIENTRY ilInit(void) {}
void ILAPIENTRY ilShutDown(void) {}
void ILAPIENTRY ilGenImages(ILsizei Num, ILuint *Images) { if (Images && Num > 0) *Images = 1; }
void ILAPIENTRY ilDeleteImages(ILsizei Num, const ILuint *Images) {}
void ILAPIENTRY ilBindImage(ILuint Image) {}
ILboolean ILAPIENTRY ilEnable(ILenum Mode) { return IL_TRUE; }
ILboolean ILAPIENTRY ilOriginFunc(ILenum Mode) { return IL_TRUE; }
ILboolean ILAPIENTRY ilLoad(ILenum Type, ILconst_string FileName) { return IL_FALSE; }
ILboolean ILAPIENTRY ilLoadL(ILenum Type, const void *Lump, ILuint Size) { return IL_FALSE; }
ILboolean ILAPIENTRY ilLoadF(ILenum Type, ILHANDLE File) { return IL_FALSE; }
ILboolean ILAPIENTRY ilSave(ILenum Type, ILconst_string FileName) { return IL_FALSE; }
ILint ILAPIENTRY ilGetInteger(ILenum Mode) { return 0; }
ILboolean ILAPIENTRY ilConvertImage(ILenum DestFormat, ILenum DestType) { return IL_TRUE; }
ILuint ILAPIENTRY ilCopyPixels(ILuint XOff, ILuint YOff, ILuint ZOff, ILuint Width, ILuint Height, ILuint Depth, ILenum Format, ILenum Type, void *Data) { return 0; }
ILboolean ILAPIENTRY ilTexImage(ILuint Width, ILuint Height, ILuint Depth, ILubyte Bpp, ILenum Format, ILenum Type, void *Data) { return IL_TRUE; }
void ILAPIENTRY ilSetPixels(ILint XOff, ILint YOff, ILint ZOff, ILuint Width, ILuint Height, ILuint Depth, ILenum Format, ILenum Type, void *Data) {}
ILubyte* ILAPIENTRY ilGetData(void) { return nullptr; }
ILenum ILAPIENTRY ilGetError(void) { return IL_NO_ERROR; }

// Discord Rich Presence Stubs
void Discord_Initialize(const char* applicationId, void* handlers, int autoRegister, const char* optionalSteamId) {}
void Discord_UpdatePresence(const void* presence) {}
void Discord_ClearPresence(void) {}
void Discord_Shutdown(void) {}
void Discord_RunCallbacks(void) {}

// libjpeg Compression & Decompression Stubs
void* jpeg_std_error(void* err) { return err; }
void jpeg_CreateCompress(void* cinfo, int version, size_t struct_size) {}
void jpeg_stdio_dest(void* cinfo, void* outfile) {}
void jpeg_set_defaults(void* cinfo) {}
void jpeg_set_quality(void* cinfo, int quality, int force_baseline) {}
void jpeg_suppress_tables(void* cinfo, int suppress) {}
void jpeg_start_compress(void* cinfo, int write_all_tables) {}
unsigned int jpeg_write_scanlines(void* cinfo, void* scanlines, unsigned int num_lines) { return num_lines; }
void jpeg_finish_compress(void* cinfo) {}
void jpeg_destroy_compress(void* cinfo) {}
void jpeg_CreateDecompress(void* cinfo, int version, size_t struct_size) {}
void jpeg_stdio_src(void* cinfo, void* infile) {}
int jpeg_read_header(void* cinfo, int require_image) { return 1; }
int jpeg_start_decompress(void* cinfo) { return 1; }
unsigned int jpeg_read_scanlines(void* cinfo, void* scanlines, unsigned int max_lines) { return 0; }
int jpeg_finish_decompress(void* cinfo) { return 1; }
void jpeg_destroy_decompress(void* cinfo) {}
int jpeg_resync_to_restart(void* cinfo, int desired) { return 1; }

// Python 2.7 Git Build Info & UCS2 Format Bridge
// Built-in Python 2.7 posixpath module so lib/os.pyc works without posixpath.pyc
static const char* s_posixpath_code = R"PY(
import os, sys, stat, genericpath
from genericpath import *

curdir = '.'
pardir = '..'
extsep = '.'
sep = '/'
pathsep = ':'
defpath = ':/bin:/usr/bin'
altsep = '\\'
devnull = '/dev/null'
supports_unicode_filenames = False

def normcase(s):
    return s

def isabs(s):
    return s.startswith('/')

def join(a, *p):
    path = a
    for b in p:
        if b.startswith('/'):
            path = b
        elif path == '' or path.endswith('/'):
            path += b
        else:
            path += '/' + b
    return path

def split(p):
    p = p.replace('\\', '/')
    i = p.rfind('/') + 1
    head, tail = p[:i], p[i:]
    if head and head != '/' * len(head):
        head = head.rstrip('/')
    return head, tail

def splitext(p):
    return genericpath._splitext(p, sep, altsep, extsep)

def splitdrive(p):
    return '', p

def basename(p):
    p = p.replace('\\', '/')
    i = p.rfind('/') + 1
    return p[i:]

def dirname(p):
    p = p.replace('\\', '/')
    i = p.rfind('/') + 1
    head = p[:i]
    if head and head != '/' * len(head):
        head = head.rstrip('/')
    return head

def islink(path):
    try:
        st = os.lstat(path)
    except (os.error, AttributeError):
        return False
    return stat.S_ISLNK(st.st_mode)

def lexists(path):
    try:
        os.lstat(path)
    except os.error:
        return False
    return True

def samefile(f1, f2):
    s1 = os.stat(f1)
    s2 = os.stat(f2)
    return samestat(s1, s2)

def sameopenfile(fp1, fp2):
    s1 = os.fstat(fp1)
    s2 = os.fstat(fp2)
    return samestat(s1, s2)

def samestat(s1, s2):
    return s1.st_ino == s2.st_ino and s1.st_dev == s2.st_dev

def ismount(path):
    if islink(path):
        return False
    try:
        s1 = os.lstat(path)
        s2 = os.lstat(join(path, '..'))
    except os.error:
        return False
    return s1.st_dev != s2.st_dev or s1.st_ino == s2.st_ino

def walk(top, func, arg):
    try:
        names = os.listdir(top)
    except os.error:
        return
    func(arg, top, names)
    for name in names:
        name = join(top, name)
        try:
            st = os.lstat(name)
        except os.error:
            continue
        if stat.S_ISDIR(st.st_mode):
            walk(name, func, arg)

def expanduser(path):
    return path

def expandvars(path):
    return path

def normpath(path):
    path = path.replace('\\', '/')
    if path == '':
        return '.'
    initial_slashes = path.startswith('/')
    if initial_slashes and path.startswith('//') and not path.startswith('///'):
        initial_slashes = 2
    comps = path.split('/')
    new_comps = []
    for comp in comps:
        if comp in ('', '.'):
            continue
        if comp != '..' or (not initial_slashes and not new_comps) or (new_comps and new_comps[-1] == '..'):
            new_comps.append(comp)
        elif new_comps:
            new_comps.pop()
    comps = new_comps
    path = '/'.join(comps)
    if initial_slashes:
        path = '/' * initial_slashes + path
    return path or '.'

def abspath(path):
    if not isabs(path):
        path = join(os.getcwd(), path)
    return normpath(path)

def realpath(filename):
    return abspath(filename)

def relpath(path, start=curdir):
    if not path:
        raise ValueError('no path specified')
    start_list = [x for x in abspath(start).split(sep) if x]
    path_list = [x for x in abspath(path).split(sep) if x]
    i = len(commonprefix([start_list, path_list]))
    rel_list = [pardir] * (len(start_list) - i) + path_list[i:]
    if not rel_list:
        return curdir
    return join(*rel_list)
)PY";

void initposixpath(void) {
    PyObject* m = Py_InitModule("posixpath", NULL);
    if (!m) return;
    PyObject* d = PyModule_GetDict(m);
    PyDict_SetItemString(d, "__builtins__", PyEval_GetBuiltins());
    PyRun_String(s_posixpath_code, Py_file_input, d, d);
}
const char* _Py_gitidentifier(void) { return "default"; }
const char* _Py_gitversion(void) { return ""; }

#undef PyUnicodeUCS2_FromFormat
__attribute__((weak)) PyObject* PyUnicodeUCS2_FromFormat(const char* format, ...) {
    va_list vargs;
    va_start(vargs, format);
    PyObject* res = PyString_FromFormatV(format, vargs);
    va_end(vargs);
    return res;
}

// Granny 3D ARM Hardware Atomics
int32_t rrAtomicCAS32(volatile uint32_t* pDestVal, uint32_t* pOldVal, uint32_t newVal) {
    return __atomic_compare_exchange_n(pDestVal, pOldVal, newVal, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST) ? 1 : 0;
}

void rrAtomicStoreRelease32(volatile uint32_t* ptr, uint32_t val) {
    __atomic_store_n(ptr, val, __ATOMIC_RELEASE);
}

// MSVC floating-point symbol stub
int _fltused = 0;

static jobject s_bgmPlayer = nullptr;
static std::string s_curBgmPath;

void AndroidSetMusicVolume(float volume) {
#ifdef __APPLE__
    IOSSetMusicVolume(volume);
    return;
#else
    if (!g_pJavaVM || !s_bgmPlayer) return;
    JNIEnv* env = nullptr;
    if (g_pJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK || !env) return;
    jclass mpCls = env->FindClass("android/media/MediaPlayer");
    if (!mpCls) { env->ExceptionClear(); return; }
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    jmethodID mid = env->GetMethodID(mpCls, "setVolume", "(FF)V");
    if (mid) env->CallVoidMethod(s_bgmPlayer, mid, (jfloat)volume, (jfloat)volume);
    env->ExceptionClear();
    env->DeleteLocalRef(mpCls);
#endif
}

void AndroidStopMusic() {
#ifdef __APPLE__
    IOSStopMusic();
    return;
#else
    if (!g_pJavaVM || !s_bgmPlayer) return;
    JNIEnv* env = nullptr;
    if (g_pJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK || !env) return;
    jclass mpCls = env->FindClass("android/media/MediaPlayer");
    if (mpCls) {
        jmethodID stopMid = env->GetMethodID(mpCls, "stop", "()V");
        if (stopMid) { env->CallVoidMethod(s_bgmPlayer, stopMid); env->ExceptionClear(); }
        jmethodID relMid = env->GetMethodID(mpCls, "release", "()V");
        if (relMid) { env->CallVoidMethod(s_bgmPlayer, relMid); env->ExceptionClear(); }
        env->DeleteLocalRef(mpCls);
    }
    env->DeleteGlobalRef(s_bgmPlayer);
    s_bgmPlayer = nullptr;
    s_curBgmPath.clear();
    env->ExceptionClear();
#endif
}

void AndroidPlayAudioFile(const char* c_szFileName, bool isLoopMusic, float volume) {
#ifndef __APPLE__
    if (!g_pJavaVM) return;
#endif
    if (!c_szFileName || !*c_szFileName) return;
    if (volume <= 0.001f) {
        if (isLoopMusic) AndroidSetMusicVolume(0.0f);
        return;
    }
    if (volume > 1.0f) volume = 1.0f;

    // Rate-limit sound effects BEFORE any EterPack decompression or file I/O!
    DWORD nowMs = ELTimer_GetMSec();
    if (!isLoopMusic) {
        static DWORD s_lastFrameMs = 0;
        static int s_soundsThisWindow = 0;
        if (nowMs - s_lastFrameMs > 30) {
            s_lastFrameMs = nowMs;
            s_soundsThisWindow = 0;
        }
        if (++s_soundsThisWindow > 3) return;
    }

#ifndef __APPLE__
    JNIEnv* env = nullptr;
    if (g_pJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK || !env) return;
#endif

    const std::string baseDir = std::string(GetMobileBaseDir()) + "/";
    std::string origPath = c_szFileName;
    for (char& c : origPath) if (c == '\\') c = '/';
    std::string lowerPath = origPath;
    for (char& c : lowerPath) c = (char)tolower((unsigned char)c);

    static std::unordered_map<std::string, std::string> s_resolvedPaths;
    std::string diskPath;
    auto itPath = s_resolvedPaths.find(lowerPath);
    if (itPath != s_resolvedPaths.end()) {
        diskPath = itPath->second;
    } else {
        // Throttle new audio file disk extractions to at most 1 per 60ms to prevent lag-spike I/O freezes
        static DWORD s_lastExtractMs = 0;
        if (!isLoopMusic && (nowMs - s_lastExtractMs < 60))
            return;
        s_lastExtractMs = nowMs;

        std::string flatRel = "temp/";
        for (char c : lowerPath) flatRel += (isalnum((unsigned char)c) || c == '.') ? c : '_';
        std::string flatAbs = baseDir + flatRel;

        if (access(flatAbs.c_str(), F_OK) == 0) {
            diskPath = flatAbs;
        } else if (access((baseDir + origPath).c_str(), F_OK) == 0) {
            diskPath = baseDir + origPath;
        } else if (access((baseDir + lowerPath).c_str(), F_OK) == 0) {
            diskPath = baseDir + lowerPath;
        } else {
            CMappedFile file;
            LPCVOID pData = nullptr;
            std::string dPath = "d:/ymir work/" + lowerPath;
            if (CEterPackManager::Instance().Get(file, c_szFileName, &pData) ||
                CEterPackManager::Instance().Get(file, lowerPath.c_str(), &pData) ||
                CEterPackManager::Instance().Get(file, dPath.c_str(), &pData)) {
                if (pData && file.Size() > 0 && file.Size() < (isLoopMusic ? (32 * 1024 * 1024) : (512 * 1024))) {
                    if (FILE* fp = fopen(flatAbs.c_str(), "wb")) {
                        fwrite(pData, 1, file.Size(), fp);
                        fclose(fp);
                        diskPath = flatAbs;
                    }
                }
            }
        }
        s_resolvedPaths[lowerPath] = diskPath;
    }
    if (diskPath.empty()) return;

#ifdef __APPLE__
    IOSPlayAudioFile(diskPath.c_str(), isLoopMusic, volume);
    return;
#else

    if (isLoopMusic) {
        if (s_bgmPlayer && s_curBgmPath == diskPath) {
            AndroidSetMusicVolume(volume);
            return;
        }
        if (s_bgmPlayer) {
            AndroidStopMusic();
        }
        jclass mpCls = env->FindClass("android/media/MediaPlayer");
        if (!mpCls) { env->ExceptionClear(); return; }
        jobject mp = env->NewObject(mpCls, env->GetMethodID(mpCls, "<init>", "()V"));
        if (env->ExceptionCheck() || !mp) {
            env->ExceptionClear();
            env->DeleteLocalRef(mpCls);
            return;
        }
        jstring jpath = env->NewStringUTF(diskPath.c_str());
        env->CallVoidMethod(mp, env->GetMethodID(mpCls, "setDataSource", "(Ljava/lang/String;)V"), jpath);
        bool err = env->ExceptionCheck();
        env->ExceptionClear();
        env->DeleteLocalRef(jpath);

        if (!err) {
            env->CallVoidMethod(mp, env->GetMethodID(mpCls, "prepare", "()V"));
            err = env->ExceptionCheck();
            env->ExceptionClear();
        }
        if (!err) {
            env->CallVoidMethod(mp, env->GetMethodID(mpCls, "setLooping", "(Z)V"), (jboolean)JNI_TRUE);
            env->ExceptionClear();
            env->CallVoidMethod(mp, env->GetMethodID(mpCls, "setVolume", "(FF)V"), (jfloat)volume, (jfloat)volume);
            env->ExceptionClear();
            env->CallVoidMethod(mp, env->GetMethodID(mpCls, "start", "()V"));
            env->ExceptionClear();
            s_bgmPlayer = env->NewGlobalRef(mp);
            s_curBgmPath = diskPath;
        } else {
            env->CallVoidMethod(mp, env->GetMethodID(mpCls, "release", "()V"));
            env->ExceptionClear();
        }
        env->DeleteLocalRef(mp);
        env->DeleteLocalRef(mpCls);
    } else {
        struct SSfxEntry { int soundId; DWORD loadTimeMs; DWORD lastPlayMs; };
        static std::unordered_map<std::string, SSfxEntry> s_sfxMap;
        static jobject s_soundPool = nullptr;
        static jmethodID s_loadMid = nullptr;
        static jmethodID s_playMid = nullptr;
        static jmethodID s_unloadMid = nullptr;

        if (!s_soundPool) {
            jclass spCls = env->FindClass("android/media/SoundPool");
            if (!spCls) { env->ExceptionClear(); return; }
            jmethodID spCtor = env->GetMethodID(spCls, "<init>", "(III)V");
            jobject sp = env->NewObject(spCls, spCtor, 8, 3, 0);
            if (env->ExceptionCheck() || !sp) {
                env->ExceptionClear();
                env->DeleteLocalRef(spCls);
                return;
            }
            s_soundPool = env->NewGlobalRef(sp);
            s_loadMid = env->GetMethodID(spCls, "load", "(Ljava/lang/String;I)I");
            s_playMid = env->GetMethodID(spCls, "play", "(IFFIIF)I");
            s_unloadMid = env->GetMethodID(spCls, "unload", "(I)Z");
            env->DeleteLocalRef(sp);
            env->DeleteLocalRef(spCls);
        }

        auto it = s_sfxMap.find(diskPath);
        if (it == s_sfxMap.end()) {
            // Prevent SoundPool 1 MB internal AudioFlinger heap exhaustion by evicting stale samples
            if (s_sfxMap.size() >= 48 && s_unloadMid) {
                for (auto& kv : s_sfxMap) {
                    if (kv.second.soundId > 0)
                        env->CallBooleanMethod(s_soundPool, s_unloadMid, (jint)kv.second.soundId);
                }
                env->ExceptionClear();
                s_sfxMap.clear();
            }
            jstring jpath = env->NewStringUTF(diskPath.c_str());
            int id = (int)env->CallIntMethod(s_soundPool, s_loadMid, jpath, 1);
            env->ExceptionClear();
            env->DeleteLocalRef(jpath);
            s_sfxMap[diskPath] = { id, nowMs, 0 };
            return;
        }

        SSfxEntry& entry = it->second;
        if (entry.soundId <= 0) return;
        if (nowMs - entry.loadTimeMs < 80) return;
        if (nowMs - entry.lastPlayMs < 75) return;
        entry.lastPlayMs = nowMs;

        env->CallIntMethod(s_soundPool, s_playMid, (jint)entry.soundId, (jfloat)volume, (jfloat)volume, 1, 0, 1.0f);
        env->ExceptionClear();
    }
#endif
}

} // extern "C"

// -----------------------------------------------------------------------------
// Miles Sound System (mss32.lib / AIL_*) & Granny Oodle0 Linker Alias Stubs
// -----------------------------------------------------------------------------
#ifdef __APPLE__
#define STUB_ZERO(name) \
    extern "C" intptr_t __android_stub_##name() __asm__("_" #name); \
    extern "C" intptr_t __android_stub_##name() { return 0; }
#else
#define STUB_ZERO(name) \
    extern "C" intptr_t __android_stub_##name() __asm__(#name); \
    extern "C" intptr_t __android_stub_##name() { return 0; }
#endif

STUB_ZERO(AIL_startup)
STUB_ZERO(AIL_shutdown)
STUB_ZERO(AIL_set_redist_directory)
STUB_ZERO(AIL_last_error)
STUB_ZERO(AIL_set_file_callbacks)
STUB_ZERO(AIL_file_read)
STUB_ZERO(AIL_file_type)
STUB_ZERO(AIL_WAV_info)
STUB_ZERO(AIL_mem_free_lock)
STUB_ZERO(AIL_decompress_ADPCM)
STUB_ZERO(AIL_decompress_ASI)

STUB_ZERO(AIL_open_digital_driver)
STUB_ZERO(AIL_close_digital_driver)
STUB_ZERO(AIL_digital_handle_release)
STUB_ZERO(AIL_digital_handle_reacquire)
STUB_ZERO(AIL_digital_CPU_percent)
STUB_ZERO(AIL_speaker_configuration)
STUB_ZERO(AIL_set_speaker_configuration)
STUB_ZERO(AIL_room_type)
STUB_ZERO(AIL_set_room_type)
STUB_ZERO(AIL_3D_room_type)
STUB_ZERO(AIL_set_3D_room_type)
STUB_ZERO(AIL_3D_speaker_type)
STUB_ZERO(AIL_set_3D_speaker_type)

STUB_ZERO(AIL_enumerate_3D_providers)
STUB_ZERO(AIL_open_3D_provider)
STUB_ZERO(AIL_close_3D_provider)
STUB_ZERO(AIL_3D_provider_attribute)
STUB_ZERO(AIL_set_3D_provider_preference)
STUB_ZERO(AIL_open_3D_listener)
STUB_ZERO(AIL_close_3D_listener)

STUB_ZERO(AIL_allocate_sample_handle)
STUB_ZERO(AIL_release_sample_handle)
STUB_ZERO(AIL_init_sample)
STUB_ZERO(AIL_set_sample_file)
STUB_ZERO(AIL_set_named_sample_file)
STUB_ZERO(AIL_sample_status)
STUB_ZERO(AIL_set_sample_loop_count)
STUB_ZERO(AIL_start_sample)
STUB_ZERO(AIL_stop_sample)
STUB_ZERO(AIL_end_sample)
STUB_ZERO(AIL_resume_sample)
STUB_ZERO(AIL_sample_volume_pan)
STUB_ZERO(AIL_set_sample_volume_pan)
STUB_ZERO(AIL_sample_volume_levels)
STUB_ZERO(AIL_set_sample_volume_levels)
STUB_ZERO(AIL_sample_ms_position)
STUB_ZERO(AIL_set_sample_ms_position)
STUB_ZERO(AIL_sample_playback_rate)
STUB_ZERO(AIL_set_sample_playback_rate)
STUB_ZERO(AIL_sample_user_data)
STUB_ZERO(AIL_set_sample_user_data)

STUB_ZERO(AIL_allocate_3D_sample_handle)
STUB_ZERO(AIL_release_3D_sample_handle)
STUB_ZERO(AIL_set_3D_sample_info)
STUB_ZERO(AIL_set_3D_sample_file)
STUB_ZERO(AIL_3D_sample_status)
STUB_ZERO(AIL_set_3D_sample_loop_count)
STUB_ZERO(AIL_start_3D_sample)
STUB_ZERO(AIL_stop_3D_sample)
STUB_ZERO(AIL_end_3D_sample)
STUB_ZERO(AIL_resume_3D_sample)
STUB_ZERO(AIL_3D_sample_volume)
STUB_ZERO(AIL_set_3D_sample_volume)
STUB_ZERO(AIL_3D_position)
STUB_ZERO(AIL_set_3D_position)
STUB_ZERO(AIL_3D_orientation)
STUB_ZERO(AIL_set_3D_orientation)
STUB_ZERO(AIL_3D_velocity)
STUB_ZERO(AIL_set_3D_velocity)
STUB_ZERO(AIL_set_3D_velocity_vector)
STUB_ZERO(AIL_3D_sample_distances)
STUB_ZERO(AIL_set_3D_sample_distances)
STUB_ZERO(AIL_3D_sample_offset)
STUB_ZERO(AIL_set_3D_sample_offset)
STUB_ZERO(AIL_3D_sample_length)
STUB_ZERO(AIL_3D_sample_playback_rate)
STUB_ZERO(AIL_set_3D_sample_playback_rate)
STUB_ZERO(AIL_auto_update_3D_position)
STUB_ZERO(AIL_update_3D_position)
STUB_ZERO(AIL_set_3D_sample_effects_level)
STUB_ZERO(AIL_3D_user_data)
STUB_ZERO(AIL_set_3D_user_data)

STUB_ZERO(AIL_open_stream)
STUB_ZERO(AIL_close_stream)
STUB_ZERO(AIL_stream_status)
STUB_ZERO(AIL_set_stream_loop_count)
STUB_ZERO(AIL_start_stream)
STUB_ZERO(AIL_pause_stream)
STUB_ZERO(AIL_stream_volume_pan)
STUB_ZERO(AIL_set_stream_volume_pan)
STUB_ZERO(AIL_stream_volume_levels)
STUB_ZERO(AIL_set_stream_volume_levels)
STUB_ZERO(AIL_stream_ms_position)
STUB_ZERO(AIL_set_stream_ms_position)
STUB_ZERO(AIL_stream_playback_rate)
STUB_ZERO(AIL_set_stream_playback_rate)
STUB_ZERO(AIL_stream_user_data)
STUB_ZERO(AIL_set_stream_user_data)
STUB_ZERO(AIL_serve)


// -----------------------------------------------------------------------------
// Granny 3D PlayStation 3 Cell SPU Stubs
// -----------------------------------------------------------------------------
namespace granny {
    struct file;
    struct model;
    struct model_instance;
    struct track_mask;
    struct mirror_specification;
    struct spu_animation;
    struct spu_animation_info;
    struct spu_animation_binding;
    struct spu_animation_binding_id;
    struct spu_track_group;
    struct radspu_command_queue;
    struct controlled_spu_animation_builder;

    extern void* SPUAnimationType = nullptr;
    extern void* SPUAnimationInfoType = nullptr;
    extern void* SPUTransformTrackType = nullptr;
    extern void* SPUTrackGroupType = nullptr;

    bool FindSPUTrackGroupForModel(spu_animation const&, char const*, int&) { return false; }
    void MakeDefaultSPUAnimationBindingID(spu_animation_binding_id&, spu_animation const*, int, model const*) {}
    spu_animation_binding* AcquireSPUAnimationBindingFromID(spu_animation_binding_id&) { return nullptr; }
    spu_animation_binding* AcquireSPUAnimationBinding(spu_animation_binding*) { return nullptr; }
    void ReleaseSPUAnimationBinding(spu_animation_binding*) {}
    int GetMaximumSPUAnimationBindingCount() { return 0; }
    void SetMaximumSPUAnimationBindingCount(int) {}
    spu_animation_info* GetSPUAnimationInfo(file&) { return nullptr; }
    radspu_command_queue* InitCommandQueueSPURS(int, int, void const*, void*) { return nullptr; }
    radspu_command_queue* InitCommandQueueSPUThreads(int, int, void const*) { return nullptr; }
    void ShutdownCommandQueueSPU(radspu_command_queue*) {}
    uint64_t GetLastInsertedFenceSPU(radspu_command_queue*) { return 0; }
    uint64_t GetCurrentClearedFenceSPU(radspu_command_queue*) { return 0; }
    uint64_t InsertFenceSPU(radspu_command_queue*) { return 0; }
    bool TryInsertCommandSPU(radspu_command_queue*, int, void const*, int) { return false; }
    uint64_t InsertCommandSPU(radspu_command_queue*, int, void const*, int) { return 0; }
    void PlayControlledSPUAnimation(float, spu_animation const&, model_instance&) {}
    controlled_spu_animation_builder* BeginControlledSPUAnimation(float, spu_animation const&) { return nullptr; }
    void EndControlledSPUAnimation(controlled_spu_animation_builder*) {}
    void SetSPUTrackGroupTarget(controlled_spu_animation_builder&, int, model_instance&) {}
    void SetSPUTrackGroupTrackMask(controlled_spu_animation_builder&, int, track_mask&) {}
    void SetSPUTrackGroupModelMask(controlled_spu_animation_builder&, int, track_mask&) {}
    void SetSPUTrackGroupBasisTransform(controlled_spu_animation_builder&, int, model&, model&) {}
    void SetSPUTrackGroupMirrorSpecification(controlled_spu_animation_builder&, int, mirror_specification&) {}
    void RecomputeSPUTrackRequiredBytes(spu_track_group&) {}
}

// -----------------------------------------------------------------------------
// Android SpeedTreeRT Loader (Loads Exact PC SpeedTreeRT Cache + Wind & Lighting)
// -----------------------------------------------------------------------------
struct STreeInternalData {
    float pos[3] = {0.0f, 0.0f, 0.0f};
    float size = 650.0f, variance = 50.0f;
    float bbox[6] = {-220.0f, -220.0f, 0.0f, 220.0f, 220.0f, 650.0f};
    std::string branchTex, compositeTex, shadowTex;

    float bAlphaTest = 84.0f;
    std::vector<float> bBaseCoords, bCoords, bNormals, bTex0, bWindWeights;
    std::vector<uint8_t> bWindIndices;
    std::vector<unsigned long> bColors;
    std::vector<unsigned short> bStrip;
    unsigned short bStripLen = 0;
    const unsigned short* bStripPtr = nullptr;

    float fAlphaTest = 84.0f;
    std::vector<float> fBaseCoords, fCoords, fNormals, fTex0, fWindWeights;
    std::vector<uint8_t> fWindIndices;
    std::vector<unsigned long> fColors;
    std::vector<unsigned short> fStrip;
    unsigned short fStripLen = 0;
    const unsigned short* fStripPtr = nullptr;

    float lAlphaTest = 84.0f;
    std::vector<float> bbTableBase, bbTableWorld;
    std::vector<float> lBaseCenters, lCenters, lNormals, lWindWeights;
    std::vector<uint8_t> lWindIndices;
    std::vector<unsigned long> lColors;
    std::vector<unsigned char> lClusterIdx;
    std::vector<float> lTexStorage, lMapCoordsStorage;
    std::vector<const float*> lTexPtrs, lMapCoordsPtrs;
    DWORD lastWindUpdateMs = 0;
    float lastCamDir[2] = {0.0f, 0.0f};
    STreeInternalData* proto = nullptr;
};

static std::unordered_map<const CSpeedTreeRT*, STreeInternalData*> s_treeMap;
static std::unordered_map<uint32_t, STreeInternalData> s_pcTreeCache;
static bool s_bPcTreeCacheLoaded = false;
static float s_windMatrices[4][16] = {
    {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},
    {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},
    {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},
    {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1}
};
static float s_camDir[3] = { 0.0f, 1.0f, 0.0f };

static uint32_t HashSptBytes(const unsigned char* data, unsigned int size) {
    uint32_t h = 2166136261u;
    for (unsigned int i = 0; i < size; ++i)
        h = (h ^ data[i]) * 16777619u;
    return h;
}

static unsigned long ApplyPcTreeShading(uint32_t origCol, const float* norm, bool isBranch) {
    float nx = norm ? norm[0] : 0.0f;
    float ny = norm ? norm[1] : 0.0f;
    float nz = norm ? norm[2] : 1.0f;
    float len = sqrtf(nx * nx + ny * ny + nz * nz);
    if (len > 0.001f) { nx /= len; ny /= len; nz /= len; }
    // Match PC sun direction & shadowmapTreePS / SetLight shading contrast
    float ndotl = nx * 0.55f + ny * 0.35f + nz * 0.75f;
    if (ndotl < 0.0f) ndotl = 0.0f;
    float shade = isBranch ? (0.24f + 0.50f * ndotl) : (0.38f + 0.52f * ndotl);
    if (shade > 1.0f) shade = 1.0f;
    uint32_t r = (uint32_t)(((origCol >> 16) & 0xFF) * shade);
    uint32_t g = (uint32_t)(((origCol >> 8)  & 0xFF) * shade);
    uint32_t b = (uint32_t)(( origCol        & 0xFF) * shade);
    if (r > 255) r = 255; if (g > 255) g = 255; if (b > 255) b = 255;
    return 0xFF000000u | (r << 16) | (g << 8) | b;
}

static void EnsurePcTreeCacheLoaded() {
    if (s_bPcTreeCacheLoaded) return;
    s_bPcTreeCacheLoaded = true;
    FILE* fp = fopen("speedtree_cache.bin", "rb");
    if (!fp) {
        TraceError("[SpeedTree] speedtree_cache.bin not found in working directory!");
        return;
    }
    uint32_t magic = 0, count = 0;
    if (fread(&magic, 4, 1, fp) != 1 || magic != 0x53505431u || fread(&count, 4, 1, fp) != 1) {
        fclose(fp);
        return;
    }
    auto ReadStr = [&](std::string& out) {
        uint16_t len = 0;
        if (fread(&len, 2, 1, fp) != 1) return;
        out.resize(len);
        if (len > 0) fread(&out[0], 1, len, fp);
    };
    for (uint32_t idx = 0; idx < count; ++idx) {
        uint32_t h = 0, uSize = 0;
        if (fread(&h, 4, 1, fp) != 1 || fread(&uSize, 4, 1, fp) != 1) break;
        STreeInternalData d;
        fread(&d.size, 4, 1, fp);
        fread(&d.variance, 4, 1, fp);
        fread(d.bbox, sizeof(float), 6, fp);
        ReadStr(d.branchTex);
        ReadStr(d.compositeTex);
        ReadStr(d.shadowTex);

        auto ReadIndexed = [&](float& alphaTest, std::vector<float>& baseC, std::vector<float>& coords,
                               std::vector<float>& norms, std::vector<unsigned long>& cols,
                               std::vector<float>& tex0, std::vector<float>& ww,
                               std::vector<uint8_t>& wi, std::vector<unsigned short>& strip,
                               unsigned short& stripLen, bool isBranch) {
            uint16_t vCount = 0, iCount = 0;
            fread(&alphaTest, 4, 1, fp);
            fread(&vCount, 2, 1, fp);
            fread(&iCount, 2, 1, fp);
            if (vCount > 0 && iCount > 0) {
                baseC.resize(vCount * 3);
                norms.resize(vCount * 3);
                std::vector<uint32_t> rawCols(vCount);
                tex0.resize(vCount * 2);
                ww.resize(vCount);
                wi.resize(vCount);
                strip.resize(iCount);
                fread(baseC.data(), sizeof(float), vCount * 3, fp);
                fread(norms.data(), sizeof(float), vCount * 3, fp);
                fread(rawCols.data(), sizeof(uint32_t), vCount, fp);
                fread(tex0.data(), sizeof(float), vCount * 2, fp);
                fread(ww.data(), sizeof(float), vCount, fp);
                fread(wi.data(), 1, vCount, fp);
                fread(strip.data(), sizeof(uint16_t), iCount, fp);
                coords = baseC;
                cols.resize(vCount);
                for (uint16_t k = 0; k < vCount; ++k)
                    cols[k] = ApplyPcTreeShading(rawCols[k], &norms[k * 3], isBranch);
                stripLen = iCount;
            }
        };

        ReadIndexed(d.bAlphaTest, d.bBaseCoords, d.bCoords, d.bNormals, d.bColors, d.bTex0, d.bWindWeights, d.bWindIndices, d.bStrip, d.bStripLen, true);
        ReadIndexed(d.fAlphaTest, d.fBaseCoords, d.fCoords, d.fNormals, d.fColors, d.fTex0, d.fWindWeights, d.fWindIndices, d.fStrip, d.fStripLen, false);

        uint16_t lCount = 0, bbCount = 0;
        fread(&d.lAlphaTest, 4, 1, fp);
        fread(&lCount, 2, 1, fp);
        fread(&bbCount, 2, 1, fp);
        if (bbCount > 0) {
            d.bbTableBase.resize(bbCount);
            fread(d.bbTableBase.data(), sizeof(float), bbCount, fp);
            d.bbTableWorld = d.bbTableBase;
        }
        if (lCount > 0) {
            d.lBaseCenters.resize(lCount * 3);
            d.lNormals.resize(lCount * 3);
            std::vector<uint32_t> rawCols(lCount);
            d.lClusterIdx.resize(lCount);
            d.lWindWeights.resize(lCount);
            d.lWindIndices.resize(lCount);
            d.lTexStorage.resize(lCount * 8);
            d.lMapCoordsStorage.resize(lCount * 16);

            fread(d.lBaseCenters.data(), sizeof(float), lCount * 3, fp);
            fread(d.lNormals.data(), sizeof(float), lCount * 3, fp);
            fread(rawCols.data(), sizeof(uint32_t), lCount, fp);
            fread(d.lClusterIdx.data(), 1, lCount, fp);
            fread(d.lWindWeights.data(), sizeof(float), lCount, fp);
            fread(d.lWindIndices.data(), 1, lCount, fp);
            fread(d.lTexStorage.data(), sizeof(float), lCount * 8, fp);
            fread(d.lMapCoordsStorage.data(), sizeof(float), lCount * 16, fp);

            d.lCenters = d.lBaseCenters;
            d.lColors.resize(lCount);
            for (uint16_t k = 0; k < lCount; ++k)
                d.lColors[k] = ApplyPcTreeShading(rawCols[k], &d.lNormals[k * 3], false);
        }
        s_pcTreeCache[h] = std::move(d);
    }
    fclose(fp);
    Tracenf("[SpeedTree] Loaded %zu precomputed PC tree models from speedtree_cache.bin", s_pcTreeCache.size());
}

static void FixupTreePointers(STreeInternalData* d) {
    d->bStripPtr = d->bStrip.empty() ? nullptr : d->bStrip.data();
    d->fStripPtr = d->fStrip.empty() ? nullptr : d->fStrip.data();
    size_t lCount = d->lClusterIdx.size();
    d->lTexPtrs.resize(lCount);
    d->lMapCoordsPtrs.resize(lCount);
    for (size_t i = 0; i < lCount; ++i) {
        d->lTexPtrs[i] = &d->lTexStorage[i * 8];
        d->lMapCoordsPtrs[i] = &d->lMapCoordsStorage[i * 16];
    }
}

static void UpdateTreeWindAndBillboards(STreeInternalData* d) {
    const bool bIsShadowPass = (GetCurrentGLDevice() != nullptr &&
                                GetCurrentGLDevice()->m_pCurrentRenderTargetTex != nullptr &&
                                GetCurrentGLDevice()->m_ActivePixelShaderType == PS_TYPE_SHADOW_DEPTH);

    float dx = s_camDir[0];
    float dy = s_camDir[1];
    if (bIsShadowPass && Frustum::Instance().graphicsLevel > 0) {
        // Lock tree leaf billboards to the sun/light direction during shadow depth passes
        // so tree shadows have the exact same shape regardless of camera rotation
        dx = -Frustum::Instance().treeLightDirection.x;
        dy = -Frustum::Instance().treeLightDirection.y;
    }

    float len = sqrtf(dx * dx + dy * dy);
    if (len > 0.001f) { dx /= len; dy /= len; } else { dx = 0.35f; dy = 0.35f; }

    DWORD nowMs = ELTimer_GetMSec();
    float camDiff = fabsf(dx - d->lastCamDir[0]) + fabsf(dy - d->lastCamDir[1]);
    if (camDiff < 0.002f && (nowMs - d->lastWindUpdateMs < 15) && !d->bbTableWorld.empty())
        return;

    d->lastWindUpdateMs = nowMs;
    d->lastCamDir[0] = dx;
    d->lastCamDir[1] = dy;

    float rx = dy, ry = -dx;
    float tSec = nowMs * 0.001f;

    size_t numCorners = d->bbTableBase.size() / 4;
    if (d->bbTableWorld.size() != d->bbTableBase.size())
        d->bbTableWorld = d->bbTableBase;

    for (size_t i = 0; i < numCorners; ++i) {
        size_t cluster = i / 4;
        float rock = sinf(tSec * 2.0f + (float)cluster * 1.7f) * 0.06f;
        float ca = cosf(rock), sa = sinf(rock);
        float ox = d->bbTableBase[i * 4 + 0];
        float oy = d->bbTableBase[i * 4 + 1];
        float oz = d->bbTableBase[i * 4 + 2];
        float rox = ox * ca - oz * sa;
        float roz = ox * sa + oz * ca;
        d->bbTableWorld[i * 4 + 0] = rox * rx + oy * dx;
        d->bbTableWorld[i * 4 + 1] = rox * ry + oy * dy;
        d->bbTableWorld[i * 4 + 2] = roz;
        d->bbTableWorld[i * 4 + 3] = 0.0f;
    }

    // 2. Apply wind matrix interpolation to leaf cluster centers
    size_t lCount = d->lClusterIdx.size();
    for (size_t i = 0; i < lCount; ++i) {
        float w = (i < d->lWindWeights.size()) ? d->lWindWeights[i] : 0.0f;
        uint8_t mIdx = (i < d->lWindIndices.size()) ? (d->lWindIndices[i] & 3) : 0;
        const float* m = s_windMatrices[mIdx];
        float x = d->lBaseCenters[i * 3 + 0];
        float y = d->lBaseCenters[i * 3 + 1];
        float z = d->lBaseCenters[i * 3 + 2];
        float wx = x * m[0] + y * m[4] + z * m[8];
        float wy = x * m[1] + y * m[5] + z * m[9];
        float wz = x * m[2] + y * m[6] + z * m[10];
        d->lCenters[i * 3 + 0] = x + (wx - x) * w;
        d->lCenters[i * 3 + 1] = y + (wy - y) * w;
        d->lCenters[i * 3 + 2] = z + (wz - z) * w;
    }
}

static STreeInternalData* GetTreeData(const CSpeedTreeRT* t) {
    auto it = s_treeMap.find(t);
    if (it != s_treeMap.end()) return it->second;
    STreeInternalData* d = new STreeInternalData();
    s_treeMap[t] = d;
    return d;
}

CSpeedTreeRT::STextures::STextures() { memset(this, 0, sizeof(*this)); }
CSpeedTreeRT::STextures::~STextures() {}
CSpeedTreeRT::SGeometry::SIndexed::SIndexed() { memset(this, 0, sizeof(*this)); }
CSpeedTreeRT::SGeometry::SIndexed::~SIndexed() {}
CSpeedTreeRT::SGeometry::SLeaf::SLeaf() { memset(this, 0, sizeof(*this)); }
CSpeedTreeRT::SGeometry::SLeaf::~SLeaf() {}
CSpeedTreeRT::SGeometry::SBillboard::SBillboard() { memset(this, 0, sizeof(*this)); }
CSpeedTreeRT::SGeometry::SBillboard::~SBillboard() {}
CSpeedTreeRT::SGeometry::SGeometry() { memset(this, 0, sizeof(*this)); }
CSpeedTreeRT::SGeometry::~SGeometry() {}

CSpeedTreeRT::CSpeedTreeRT() { GetTreeData(this); }
CSpeedTreeRT::~CSpeedTreeRT() {
    auto it = s_treeMap.find(this);
    if (it != s_treeMap.end()) { delete it->second; s_treeMap.erase(it); }
}

void* CSpeedTreeRT::operator new(size_t size) { return ::operator new(size); }
void CSpeedTreeRT::operator delete(void* ptr) { ::operator delete(ptr); }

bool CSpeedTreeRT::LoadTree(const char*) { return true; }
bool CSpeedTreeRT::LoadTree(const unsigned char* pBlock, unsigned int nNumBytes) {
    if (!pBlock || nNumBytes < 16) return false;
    EnsurePcTreeCacheLoaded();
    uint32_t h = HashSptBytes(pBlock, nNumBytes);
    auto it = s_pcTreeCache.find(h);
    if (it != s_pcTreeCache.end()) {
        STreeInternalData* dst = GetTreeData(this);
        *dst = it->second;
        FixupTreePointers(dst);
        return true;
    }
    // Fallback: match by embedded texture filename inside pBlock
    for (const auto& kv : s_pcTreeCache) {
        if (kv.second.branchTex.empty()) continue;
        std::string stem = kv.second.branchTex;
        size_t dot = stem.find_last_of('.');
        if (dot != std::string::npos) stem = stem.substr(0, dot);
        if (stem.size() >= 4) {
            for (unsigned int i = 0; i + stem.size() < nNumBytes; ++i) {
                if (strncasecmp((const char*)(pBlock + i), stem.c_str(), stem.size()) == 0) {
                    STreeInternalData* dst = GetTreeData(this);
                    *dst = kv.second;
                    FixupTreePointers(dst);
                    return true;
                }
            }
        }
    }
    if (!s_pcTreeCache.empty()) {
        STreeInternalData* dst = GetTreeData(this);
        *dst = s_pcTreeCache.begin()->second;
        FixupTreePointers(dst);
        return true;
    }
    return false;
}
void CSpeedTreeRT::GetTreeSize(float& fSize, float& fVariance) const {
    STreeInternalData* d = GetTreeData(this); fSize = d->size; fVariance = d->variance;
}
void CSpeedTreeRT::SetTreeSize(float fNewSize, float fNewVariance) {
    STreeInternalData* d = GetTreeData(this);
    if (fNewSize > 10.0f) d->size = fNewSize;
    d->variance = fNewVariance;
}
void CSpeedTreeRT::GetBoundingBox(float* pBounds) const {
    if (pBounds) memcpy(pBounds, GetTreeData(this)->bbox, sizeof(float) * 6);
}
const float* CSpeedTreeRT::GetTreePosition() const { return GetTreeData(this)->pos; }
void CSpeedTreeRT::SetTreePosition(float x, float y, float z) {
    STreeInternalData* d = GetTreeData(this); d->pos[0] = x; d->pos[1] = y; d->pos[2] = z;
}

void CSpeedTreeRT::SetBranchLightingMethod(ELightingMethod) {}
void CSpeedTreeRT::SetLeafLightingMethod(ELightingMethod) {}
void CSpeedTreeRT::SetFrondLightingMethod(ELightingMethod) {}
void CSpeedTreeRT::SetBranchWindMethod(EWindMethod) {}
void CSpeedTreeRT::SetLeafWindMethod(EWindMethod) {}
void CSpeedTreeRT::SetFrondWindMethod(EWindMethod) {}
void CSpeedTreeRT::SetNumLeafRockingGroups(unsigned int) {}
void CSpeedTreeRT::SetTextureFlip(bool) {}
void CSpeedTreeRT::SetLeafRockingState(bool) {}
const float* CSpeedTreeRT::GetBranchMaterial() const { static float s_mat[13] = {1,1,1,1, 0.5f,0.5f,0.5f,1, 0,0,0,1, 0}; return s_mat; }
const float* CSpeedTreeRT::GetFrondMaterial() const { static float s_mat[13] = {1,1,1,1, 0.5f,0.5f,0.5f,1, 0,0,0,1, 0}; return s_mat; }
const float* CSpeedTreeRT::GetLeafMaterial() const { static float s_mat[13] = {1,1,1,1, 0.5f,0.5f,0.5f,1, 0,0,0,1, 0}; return s_mat; }
float CSpeedTreeRT::GetLeafLightingAdjustment() const { return 1.0f; }
void CSpeedTreeRT::SetNumWindMatrices(unsigned int) {}
void CSpeedTreeRT::SetWindMatrix(unsigned int nMatrixIndex, const float* pMatrix) {
    if (nMatrixIndex < 4 && pMatrix)
        memcpy(s_windMatrices[nMatrixIndex], pMatrix, 16 * sizeof(float));
}
void CSpeedTreeRT::SetTime(float) {}
float CSpeedTreeRT::SetWindStrength(float, float, float) { return 0.0f; }
void CSpeedTreeRT::SetLightAttributes(unsigned int, const float*) {}
void CSpeedTreeRT::SetLightState(unsigned int, bool) {}
void CSpeedTreeRT::SetCamera(const float*, const float* pCamDir) {
    if (pCamDir) {
        s_camDir[0] = pCamDir[0];
        s_camDir[1] = pCamDir[1];
        s_camDir[2] = pCamDir[2];
    }
}
void CSpeedTreeRT::SetLocalMatrices(unsigned int, unsigned int) {}
void CSpeedTreeRT::SetDropToBillboard(bool) {}
const char* CSpeedTreeRT::GetCurrentError() { return ""; }

bool CSpeedTreeRT::Compute(const float*, unsigned int, bool) {
    STreeInternalData* d = GetTreeData(this);
    FixupTreePointers(d);
    return !d->branchTex.empty();
}
void CSpeedTreeRT::ComputeLodLevel() {}
float CSpeedTreeRT::GetLodLevel() const { return 1.0f; }
void CSpeedTreeRT::SetLodLevel(float) {}

void CSpeedTreeRT::GetGeometry(SGeometry& sGeometry, unsigned long ulBitVector, short, short, short) {
    STreeInternalData* inst = GetTreeData(this);
    STreeInternalData* d = inst->proto ? inst->proto : inst;
    if (ulBitVector & SpeedTree_LeafGeometry)
        UpdateTreeWindAndBillboards(d);

    sGeometry.m_fBranchAlphaTestValue = d->bAlphaTest;
    sGeometry.m_sBranches.m_nDiscreteLodLevel = 0;
    sGeometry.m_sBranches.m_usNumStrips = d->bStripLen > 0 ? 1 : 0;
    sGeometry.m_sBranches.m_pStripLengths = &d->bStripLen;
    sGeometry.m_sBranches.m_pStrips = &d->bStripPtr;
    sGeometry.m_sBranches.m_usVertexCount = (unsigned short)(d->bCoords.size() / 3);
    sGeometry.m_sBranches.m_pCoords = d->bCoords.data();
    sGeometry.m_sBranches.m_pNormals = d->bNormals.data();
    sGeometry.m_sBranches.m_pTexCoords0 = d->bTex0.data();
    sGeometry.m_sBranches.m_pColors = d->bColors.data();
    sGeometry.m_sBranches.m_pWindWeights = d->bWindWeights.data();
    sGeometry.m_sBranches.m_pWindMatrixIndices = d->bWindIndices.data();

    sGeometry.m_fFrondAlphaTestValue = d->fAlphaTest;
    sGeometry.m_sFronds.m_nDiscreteLodLevel = 0;
    sGeometry.m_sFronds.m_usNumStrips = d->fStripLen > 0 ? 1 : 0;
    sGeometry.m_sFronds.m_pStripLengths = &d->fStripLen;
    sGeometry.m_sFronds.m_pStrips = &d->fStripPtr;
    sGeometry.m_sFronds.m_usVertexCount = (unsigned short)(d->fCoords.size() / 3);
    sGeometry.m_sFronds.m_pCoords = d->fCoords.data();
    sGeometry.m_sFronds.m_pNormals = d->fNormals.data();
    sGeometry.m_sFronds.m_pTexCoords0 = d->fTex0.data();
    sGeometry.m_sFronds.m_pColors = d->fColors.data();
    sGeometry.m_sFronds.m_pWindWeights = d->fWindWeights.data();
    sGeometry.m_sFronds.m_pWindMatrixIndices = d->fWindIndices.data();

    sGeometry.m_sLeaves0.m_bIsActive = !d->lClusterIdx.empty();
    sGeometry.m_sLeaves0.m_nDiscreteLodLevel = 0;
    sGeometry.m_sLeaves0.m_fAlphaTestValue = d->lAlphaTest;
    sGeometry.m_sLeaves0.m_usLeafCount = (unsigned short)d->lClusterIdx.size();
    sGeometry.m_sLeaves0.m_pCenterCoords = d->lCenters.data();
    sGeometry.m_sLeaves0.m_pNormals = d->lNormals.data();
    sGeometry.m_sLeaves0.m_pColors = d->lColors.data();
    sGeometry.m_sLeaves0.m_pLeafClusterIndices = d->lClusterIdx.data();
    sGeometry.m_sLeaves0.m_pLeafMapTexCoords = d->lTexPtrs.data();
    sGeometry.m_sLeaves0.m_pLeafMapCoords = d->lMapCoordsPtrs.data();
    sGeometry.m_sLeaves0.m_pWindWeights = d->lWindWeights.data();
    sGeometry.m_sLeaves0.m_pWindMatrixIndices = d->lWindIndices.data();
}

void CSpeedTreeRT::GetTextures(STextures& sTextures) const {
    STreeInternalData* inst = GetTreeData(this);
    STreeInternalData* d = inst->proto ? inst->proto : inst;
    sTextures.m_pBranchTextureFilename = d->branchTex.c_str();
    sTextures.m_pCompositeFilename = d->compositeTex.empty() ? nullptr : d->compositeTex.c_str();
    sTextures.m_pSelfShadowFilename = d->shadowTex.empty() ? nullptr : d->shadowTex.c_str();
}

const float* CSpeedTreeRT::GetLeafBillboardTable(unsigned int& nEntryCount) const {
    STreeInternalData* inst = GetTreeData(this);
    STreeInternalData* d = inst->proto ? inst->proto : inst;
    UpdateTreeWindAndBillboards(d);
    nEntryCount = (unsigned int)d->bbTableWorld.size();
    return d->bbTableWorld.empty() ? nullptr : d->bbTableWorld.data();
}
const float* CSpeedTreeRT::GetLeafLodSizeAdjustments() { static float s_adj[4] = {1,1,1,1}; return s_adj; }
unsigned int CSpeedTreeRT::GetCollisionObjectCount() { return 0; }
void CSpeedTreeRT::GetCollisionObject(unsigned int, ECollisionObjectType&, float*, float*) {}
unsigned short CSpeedTreeRT::GetNumBranchLodLevels() const { return 1; }
unsigned short CSpeedTreeRT::GetNumFrondLodLevels() const { return 1; }
unsigned short CSpeedTreeRT::GetNumLeafLodLevels() const { return 1; }
void CSpeedTreeRT::DeleteBranchGeometry() {}
void CSpeedTreeRT::DeleteFrondGeometry() {}
void CSpeedTreeRT::DeleteTransientData() {}
CSpeedTreeRT* CSpeedTreeRT::Clone(float, float, float, unsigned int) const { return nullptr; }
const CSpeedTreeRT* CSpeedTreeRT::InstanceOf() const { return this; }
CSpeedTreeRT* CSpeedTreeRT::MakeInstance() {
    CSpeedTreeRT* inst = new CSpeedTreeRT();
    STreeInternalData* src = GetTreeData(this);
    STreeInternalData* rootProto = src->proto ? src->proto : src;
    STreeInternalData* dst = GetTreeData(inst);
    dst->proto = rootProto;
    memcpy(dst->pos, rootProto->pos, sizeof(dst->pos));
    dst->size = rootProto->size;
    dst->variance = rootProto->variance;
    memcpy(dst->bbox, rootProto->bbox, sizeof(dst->bbox));
    dst->branchTex = rootProto->branchTex;
    return inst;
}

#endif // __ANDROID__