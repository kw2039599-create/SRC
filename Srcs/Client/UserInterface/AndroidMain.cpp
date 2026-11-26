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
#define LOG_TAG "WorldOnline"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
#include <stdio.h>
typedef void JNIEnv;
typedef void* jobject;
typedef void* jclass;
typedef void* jstring;
typedef void* jmethodID;
typedef int jint;
typedef float jfloat;
typedef unsigned char jboolean;
typedef void JavaVM;
typedef void AAssetManager;
#define JNIEXPORT extern "C" __attribute__((visibility("default")))
#define JNICALL
#define LOGI(...) do { fprintf(stdout, "[WorldOnline] "); fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); } while(0)
#define LOGE(...) do { fprintf(stderr, "[WorldOnline ERR] "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while(0)

extern "C" {
    bool IOSDecodeImageFromMemory(const void* data, unsigned int size, unsigned int* outW, unsigned int* outH, std::vector<uint32_t>& outBGRA);
    bool IOSRenderCharGlyph(int pxSize, unsigned short ch, int* outW, int* outH, int* outAdv, uint32_t* outPixels64);
    void IOSToggleSoftKeyboard(bool show);
    void IOSOpenURL(const char* urlStr);
    void IOSCloseApp();
    void IOSSetMusicVolume(float volume);
    void IOSStopMusic();
    void IOSPlayAudioFile(const char* diskPath, bool isLoopMusic, float volume);
    void IOSPlayAudioFile3D(const char* diskPath, float volume, float pan, float rate);
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

#ifdef __APPLE__
typedef void* ObjcId;
typedef void* ObjcClass;
typedef void* ObjcSEL;
typedef void* ObjcMethod;
typedef void (*ObjcIMP)(void);

typedef ObjcClass (*PFN_objc_getClass)(const char*);
typedef ObjcClass (*PFN_object_getClass)(ObjcId);
typedef ObjcSEL (*PFN_sel_registerName)(const char*);
typedef ObjcMethod (*PFN_class_getInstanceMethod)(ObjcClass, ObjcSEL);
typedef ObjcIMP (*PFN_method_setImplementation)(ObjcMethod, ObjcIMP);

static PFN_objc_getClass s_objc_getClass = nullptr;
static PFN_object_getClass s_object_getClass = nullptr;
static PFN_sel_registerName s_sel_registerName = nullptr;
static PFN_class_getInstanceMethod s_class_getInstanceMethod = nullptr;
static PFN_method_setImplementation s_method_setImplementation = nullptr;
static void* s_objc_msgSend = nullptr;

static bool EnsureObjcRuntimeLoaded() {
    static bool s_bLoaded = false;
    if (s_bLoaded) return (s_objc_msgSend != nullptr);
    s_bLoaded = true;
    s_objc_getClass = (PFN_objc_getClass)dlsym(RTLD_DEFAULT, "objc_getClass");
    s_object_getClass = (PFN_object_getClass)dlsym(RTLD_DEFAULT, "object_getClass");
    s_sel_registerName = (PFN_sel_registerName)dlsym(RTLD_DEFAULT, "sel_registerName");
    s_class_getInstanceMethod = (PFN_class_getInstanceMethod)dlsym(RTLD_DEFAULT, "class_getInstanceMethod");
    s_method_setImplementation = (PFN_method_setImplementation)dlsym(RTLD_DEFAULT, "method_setImplementation");
    s_objc_msgSend = dlsym(RTLD_DEFAULT, "objc_msgSend");
    return (s_objc_getClass && s_object_getClass && s_sel_registerName &&
            s_class_getInstanceMethod && s_method_setImplementation && s_objc_msgSend);
}

static ObjcIMP s_origUITextFieldDeleteBackward = nullptr;
static ObjcIMP s_origUITextViewDeleteBackward = nullptr;
static ObjcIMP s_origCustomViewDeleteBackward = nullptr;
static ObjcClass s_hookedCustomKeyInputCls = nullptr;
static volatile uint32_t s_uIosBackspaceEventCount = 0;

extern "C" void Java_com_worldonline_client_NativeLib_keyEvent(JNIEnv* env, jobject obj, jint keyCode, jint unicodeChar);

static void Hooked_UITextField_deleteBackward(ObjcId self, ObjcSEL _cmd) {
    uint32_t before = s_uIosBackspaceEventCount;
    if (s_origUITextFieldDeleteBackward) {
        ((void(*)(ObjcId, ObjcSEL))s_origUITextFieldDeleteBackward)(self, _cmd);
    }
    if (s_uIosBackspaceEventCount == before) {
        Java_com_worldonline_client_NativeLib_keyEvent(nullptr, nullptr, 67, 8);
    }
}

static void Hooked_UITextView_deleteBackward(ObjcId self, ObjcSEL _cmd) {
    uint32_t before = s_uIosBackspaceEventCount;
    if (s_origUITextViewDeleteBackward) {
        ((void(*)(ObjcId, ObjcSEL))s_origUITextViewDeleteBackward)(self, _cmd);
    }
    if (s_uIosBackspaceEventCount == before) {
        Java_com_worldonline_client_NativeLib_keyEvent(nullptr, nullptr, 67, 8);
    }
}

static void Hooked_CustomView_deleteBackward(ObjcId self, ObjcSEL _cmd) {
    uint32_t before = s_uIosBackspaceEventCount;
    if (s_origCustomViewDeleteBackward) {
        ((void(*)(ObjcId, ObjcSEL))s_origCustomViewDeleteBackward)(self, _cmd);
    }
    if (s_uIosBackspaceEventCount == before) {
        Java_com_worldonline_client_NativeLib_keyEvent(nullptr, nullptr, 67, 8);
    }
}

static bool Hooked_UIView_isMultipleTouchEnabled(ObjcId /*self*/, ObjcSEL /*_cmd*/) {
    return true;
}

static void EnableIosMultipleTouchOnViewHierarchy(ObjcId view) {
    if (!view || !EnsureObjcRuntimeLoaded()) return;
    ObjcSEL respSel = s_sel_registerName("respondsToSelector:");
    ObjcSEL setMultiSel = s_sel_registerName("setMultipleTouchEnabled:");
    if (((bool(*)(ObjcId, ObjcSEL, ObjcSEL))s_objc_msgSend)(view, respSel, setMultiSel)) {
        ((void(*)(ObjcId, ObjcSEL, bool))s_objc_msgSend)(view, setMultiSel, true);
    }
    ObjcSEL delSel = s_sel_registerName("deleteBackward");
    if (!s_hookedCustomKeyInputCls && ((bool(*)(ObjcId, ObjcSEL, ObjcSEL))s_objc_msgSend)(view, respSel, delSel)) {
        ObjcClass cls = s_object_getClass(view);
        if (cls && cls != s_objc_getClass("UITextField") && cls != s_objc_getClass("UITextView")) {
            if (ObjcMethod m = s_class_getInstanceMethod(cls, delSel)) {
                s_hookedCustomKeyInputCls = cls;
                s_origCustomViewDeleteBackward = s_method_setImplementation(m, (ObjcIMP)Hooked_CustomView_deleteBackward);
            }
        }
    }
    ObjcId subviews = ((ObjcId(*)(ObjcId, ObjcSEL))s_objc_msgSend)(view, s_sel_registerName("subviews"));
    if (subviews) {
        unsigned long count = ((unsigned long(*)(ObjcId, ObjcSEL))s_objc_msgSend)(subviews, s_sel_registerName("count"));
        ObjcSEL objAtIdxSel = s_sel_registerName("objectAtIndex:");
        for (unsigned long i = 0; i < count; ++i) {
            ObjcId child = ((ObjcId(*)(ObjcId, ObjcSEL, unsigned long))s_objc_msgSend)(subviews, objAtIdxSel, i);
            EnableIosMultipleTouchOnViewHierarchy(child);
        }
    }
}

static void InstallIosInputHooks(bool forceRefreshHierarchy = false) {
    if (!EnsureObjcRuntimeLoaded()) return;
    static bool s_bInstalled = false;
    if (!s_bInstalled) {
        s_bInstalled = true;
        ObjcSEL delSel = s_sel_registerName("deleteBackward");
        if (ObjcClass tfCls = s_objc_getClass("UITextField")) {
            if (ObjcMethod m = s_class_getInstanceMethod(tfCls, delSel)) {
                s_origUITextFieldDeleteBackward = s_method_setImplementation(m, (ObjcIMP)Hooked_UITextField_deleteBackward);
            }
        }
        if (ObjcClass tvCls = s_objc_getClass("UITextView")) {
            if (ObjcMethod m = s_class_getInstanceMethod(tvCls, delSel)) {
                s_origUITextViewDeleteBackward = s_method_setImplementation(m, (ObjcIMP)Hooked_UITextView_deleteBackward);
            }
        }
        if (ObjcClass viewCls = s_objc_getClass("UIView")) {
            if (ObjcMethod m = s_class_getInstanceMethod(viewCls, s_sel_registerName("isMultipleTouchEnabled"))) {
                s_method_setImplementation(m, (ObjcIMP)Hooked_UIView_isMultipleTouchEnabled);
            }
        }
        forceRefreshHierarchy = true;
    }
    if (forceRefreshHierarchy) {
        if (ObjcClass appCls = s_objc_getClass("UIApplication")) {
            ObjcId sharedApp = ((ObjcId(*)(ObjcId, ObjcSEL))s_objc_msgSend)((ObjcId)appCls, s_sel_registerName("sharedApplication"));
            if (sharedApp) {
                ObjcId keyWin = ((ObjcId(*)(ObjcId, ObjcSEL))s_objc_msgSend)(sharedApp, s_sel_registerName("keyWindow"));
                EnableIosMultipleTouchOnViewHierarchy(keyWin);
            }
        }
    }
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
int g_AndroidRawTouchX = 640;
int g_AndroidRawTouchY = 360;
static JavaVM* g_pJavaVM = nullptr;

#ifdef ENABLE_VOICE_CHAT
} // temporarily close extern "C" for C++ VoiceChat bridge
#include <deque>
#include <pthread.h>

namespace amun {
    VoiceChat::VoiceChat() {}
    VoiceChat::~VoiceChat() {}
}

static pthread_mutex_t s_voiceMutex = PTHREAD_MUTEX_INITIALIZER;
static std::deque<std::vector<int16_t>> s_capturedVoiceQueue;
static std::deque<std::vector<int16_t>> s_playbackVoiceQueue;
static volatile bool s_bMobileVoiceInitialized = false;
static volatile bool s_bMobileVoiceRecording = false;
static volatile bool s_bMobileVoicePermGranted = false;

static void PushCapturedVoiceSamples(const int16_t* samples, size_t count) {
    if (!samples || count == 0) return;
    pthread_mutex_lock(&s_voiceMutex);
    if (s_capturedVoiceQueue.size() < 32) {
        s_capturedVoiceQueue.emplace_back(samples, samples + count);
    }
    pthread_mutex_unlock(&s_voiceMutex);
}

#if !defined(__APPLE__)
static pthread_t s_androidMicThread;
static pthread_t s_androidPlayThread;
static volatile bool s_bAndroidPlayThreadRunning = false;

static jobject GetCurrentAndroidActivity(JNIEnv* env) {
    if (!env) return nullptr;
    jclass atCls = env->FindClass("android/app/ActivityThread");
    if (!atCls) { env->ExceptionClear(); return nullptr; }
    jmethodID curAtMid = env->GetStaticMethodID(atCls, "currentActivityThread", "()Landroid/app/ActivityThread;");
    jobject atObj = curAtMid ? env->CallStaticObjectMethod(atCls, curAtMid) : nullptr;
    env->ExceptionClear();
    jobject activityObj = nullptr;
    if (atObj) {
        jfieldID mActFid = env->GetFieldID(atCls, "mActivities", "Landroid/util/ArrayMap;");
        env->ExceptionClear();
        if (!mActFid) {
            mActFid = env->GetFieldID(atCls, "mActivities", "Ljava/util/Map;");
            env->ExceptionClear();
        }
        jobject mapObj = mActFid ? env->GetObjectField(atObj, mActFid) : nullptr;
        env->ExceptionClear();
        if (mapObj) {
            jclass mapCls = env->FindClass("java/util/Map");
            jmethodID valuesMid = env->GetMethodID(mapCls, "values", "()Ljava/util/Collection;");
            jobject colObj = valuesMid ? env->CallObjectMethod(mapObj, valuesMid) : nullptr;
            env->ExceptionClear();
            if (colObj) {
                jclass colCls = env->FindClass("java/util/Collection");
                jmethodID toArrMid = env->GetMethodID(colCls, "toArray", "()[Ljava/lang/Object;");
                jobjectArray arr = toArrMid ? (jobjectArray)env->CallObjectMethod(colObj, toArrMid) : nullptr;
                env->ExceptionClear();
                if (arr) {
                    jsize len = env->GetArrayLength(arr);
                    for (jsize i = 0; i < len; ++i) {
                        jobject rec = env->GetObjectArrayElement(arr, i);
                        if (!rec) continue;
                        jclass recCls = env->GetObjectClass(rec);
                        jfieldID actFid = env->GetFieldID(recCls, "activity", "Landroid/app/Activity;");
                        jfieldID pausedFid = env->GetFieldID(recCls, "paused", "Z");
                        env->ExceptionClear();
                        if (actFid) {
                            jobject cand = env->GetObjectField(rec, actFid);
                            jboolean isPaused = pausedFid ? env->GetBooleanField(rec, pausedFid) : JNI_FALSE;
                            env->ExceptionClear();
                            if (cand) {
                                if (!isPaused || !activityObj) {
                                    if (activityObj) env->DeleteLocalRef(activityObj);
                                    activityObj = cand;
                                    if (!isPaused) {
                                        env->DeleteLocalRef(recCls);
                                        env->DeleteLocalRef(rec);
                                        break;
                                    }
                                } else {
                                    env->DeleteLocalRef(cand);
                                }
                            }
                        }
                        env->DeleteLocalRef(recCls);
                        env->DeleteLocalRef(rec);
                    }
                    env->DeleteLocalRef(arr);
                }
                env->DeleteLocalRef(colCls);
                env->DeleteLocalRef(colObj);
            }
            env->DeleteLocalRef(mapCls);
            env->DeleteLocalRef(mapObj);
        }
        env->DeleteLocalRef(atObj);
    }
    env->DeleteLocalRef(atCls);
    return activityObj;
}

static bool CheckOrRequestAndroidMicPermission() {
    if (s_bMobileVoicePermGranted) return true;
    if (!g_pJavaVM) return false;
    JNIEnv* env = nullptr;
    if (g_pJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK || !env) return false;

    jobject activity = GetCurrentAndroidActivity(env);
    if (!activity) return false;

    jclass actCls = env->GetObjectClass(activity);
    jmethodID checkMid = env->GetMethodID(actCls, "checkSelfPermission", "(Ljava/lang/String;)I");
    env->ExceptionClear();
    bool granted = true;
    if (checkMid) {
        jstring permStr = env->NewStringUTF("android.permission.RECORD_AUDIO");
        jint res = env->CallIntMethod(activity, checkMid, permStr);
        env->ExceptionClear();
        granted = (res == 0);
        if (!granted) {
            static DWORD s_dwLastPromptMs = 0;
            DWORD nowMs = ELTimer_GetMSec();
            if (nowMs - s_dwLastPromptMs > 4000) {
                s_dwLastPromptMs = nowMs;
                jmethodID reqMid = env->GetMethodID(actCls, "requestPermissions", "([Ljava/lang/String;I)V");
                env->ExceptionClear();
                if (reqMid) {
                    jclass strCls = env->FindClass("java/lang/String");
                    jobjectArray permArr = env->NewObjectArray(1, strCls, permStr);
                    env->CallVoidMethod(activity, reqMid, permArr, (jint)1001);
                    env->ExceptionClear();
                    env->DeleteLocalRef(permArr);
                    env->DeleteLocalRef(strCls);
                }
            }
        }
        env->DeleteLocalRef(permStr);
    }
    env->DeleteLocalRef(actCls);
    env->DeleteLocalRef(activity);
    s_bMobileVoicePermGranted = granted;
    return granted;
}

static void* AndroidMicCaptureThreadProc(void*) {
    if (!g_pJavaVM) return nullptr;
    JNIEnv* env = nullptr;
    if (g_pJavaVM->AttachCurrentThread(&env, nullptr) != JNI_OK || !env) return nullptr;

    jclass arCls = env->FindClass("android/media/AudioRecord");
    if (arCls) {
        jmethodID minBufMid = env->GetStaticMethodID(arCls, "getMinBufferSize", "(III)I");
        jmethodID ctorMid = env->GetMethodID(arCls, "<init>", "(IIIII)V");
        jmethodID getStateMid = env->GetMethodID(arCls, "getState", "()I");
        jmethodID startMid = env->GetMethodID(arCls, "startRecording", "()V");
        jmethodID readMid = env->GetMethodID(arCls, "read", "([SII)I");
        jmethodID stopMid = env->GetMethodID(arCls, "stop", "()V");
        jmethodID relMid = env->GetMethodID(arCls, "release", "()V");

        int rate = 32000;
        jint minBuf = env->CallStaticIntMethod(arCls, minBufMid, (jint)rate, (jint)16, (jint)2);
        env->ExceptionClear();
        if (minBuf <= 0) {
            rate = 16000;
            minBuf = env->CallStaticIntMethod(arCls, minBufMid, (jint)rate, (jint)16, (jint)2);
            env->ExceptionClear();
        }
        int bufBytes = std::max<int>((int)minBuf * 2, rate / 5);
        jobject recorder = env->NewObject(arCls, ctorMid, (jint)1 /*MIC*/, (jint)rate, (jint)16 /*MONO*/, (jint)2 /*PCM_16BIT*/, (jint)bufBytes);
        env->ExceptionClear();

        if (recorder && env->CallIntMethod(recorder, getStateMid) == 1) {
            env->CallVoidMethod(recorder, startMid);
            env->ExceptionClear();
            const int chunkSamples = (rate == 32000) ? 1600 : 800; // 50ms chunks
            jshortArray pcmArr = env->NewShortArray(chunkSamples);
            std::vector<int16_t> tempBuf(chunkSamples);
            std::vector<int16_t> upsampledBuf(1600);

            while (s_bMobileVoiceRecording) {
                jint nRead = env->CallIntMethod(recorder, readMid, pcmArr, 0, chunkSamples);
                if (env->ExceptionCheck()) { env->ExceptionClear(); break; }
                if (nRead > 0) {
                    env->GetShortArrayRegion(pcmArr, 0, nRead, tempBuf.data());
                    if (rate == 32000) {
                        PushCapturedVoiceSamples(tempBuf.data(), (size_t)nRead);
                    } else {
                        size_t outCount = (size_t)nRead * 2;
                        if (upsampledBuf.size() < outCount) upsampledBuf.resize(outCount);
                        for (int i = 0; i < nRead; ++i) {
                            upsampledBuf[i * 2] = tempBuf[i];
                            upsampledBuf[i * 2 + 1] = (i + 1 < nRead) ? (int16_t)((tempBuf[i] + tempBuf[i + 1]) / 2) : tempBuf[i];
                        }
                        PushCapturedVoiceSamples(upsampledBuf.data(), outCount);
                    }
                } else {
                    usleep(10000);
                }
            }
            env->DeleteLocalRef(pcmArr);
            env->CallVoidMethod(recorder, stopMid);
            env->ExceptionClear();
        } else {
            LOGE("[VoiceChat] Android AudioRecord failed to initialize (check RECORD_AUDIO permission)");
        }
        if (recorder) {
            env->CallVoidMethod(recorder, relMid);
            env->ExceptionClear();
            env->DeleteLocalRef(recorder);
        }
        env->DeleteLocalRef(arCls);
    }
    g_pJavaVM->DetachCurrentThread();
    return nullptr;
}

static void* AndroidVoicePlayThreadProc(void*) {
    if (!g_pJavaVM) return nullptr;
    JNIEnv* env = nullptr;
    if (g_pJavaVM->AttachCurrentThread(&env, nullptr) != JNI_OK || !env) return nullptr;

    jclass atCls = env->FindClass("android/media/AudioTrack");
    if (atCls) {
        jmethodID minBufMid = env->GetStaticMethodID(atCls, "getMinBufferSize", "(III)I");
        jmethodID ctorMid = env->GetMethodID(atCls, "<init>", "(IIIIII)V");
        jmethodID playMid = env->GetMethodID(atCls, "play", "()V");
        jmethodID writeMid = env->GetMethodID(atCls, "write", "([SII)I");
        jmethodID stopMid = env->GetMethodID(atCls, "stop", "()V");
        jmethodID relMid = env->GetMethodID(atCls, "release", "()V");

        jint minBuf = env->CallStaticIntMethod(atCls, minBufMid, (jint)32000, (jint)4 /*OUT_MONO*/, (jint)2 /*PCM_16BIT*/);
        env->ExceptionClear();
        int bufBytes = std::max<int>((int)minBuf * 2, 12800);
        jobject track = env->NewObject(atCls, ctorMid, (jint)3 /*STREAM_MUSIC*/, (jint)32000, (jint)4, (jint)2, (jint)bufBytes, (jint)1 /*MODE_STREAM*/);
        env->ExceptionClear();

        if (track) {
            env->CallVoidMethod(track, playMid);
            env->ExceptionClear();
            jshortArray outArr = env->NewShortArray(3200);

            while (s_bAndroidPlayThreadRunning) {
                std::vector<int16_t> chunk;
                pthread_mutex_lock(&s_voiceMutex);
                if (!s_playbackVoiceQueue.empty()) {
                    chunk = std::move(s_playbackVoiceQueue.front());
                    s_playbackVoiceQueue.pop_front();
                }
                pthread_mutex_unlock(&s_voiceMutex);

                if (!chunk.empty()) {
                    jsize len = (jsize)std::min<size_t>(chunk.size(), 3200);
                    env->SetShortArrayRegion(outArr, 0, len, chunk.data());
                    env->CallIntMethod(track, writeMid, outArr, 0, len);
                    env->ExceptionClear();
                } else {
                    usleep(10000);
                }
            }
            env->DeleteLocalRef(outArr);
            env->CallVoidMethod(track, stopMid);
            env->CallVoidMethod(track, relMid);
            env->ExceptionClear();
            env->DeleteLocalRef(track);
        }
        env->DeleteLocalRef(atCls);
    }
    g_pJavaVM->DetachCurrentThread();
    return nullptr;
}
#else // __APPLE__
struct IOSAudioStreamBasicDescription {
    double mSampleRate;
    uint32_t mFormatID;
    uint32_t mFormatFlags;
    uint32_t mBytesPerPacket;
    uint32_t mFramesPerPacket;
    uint32_t mBytesPerFrame;
    uint32_t mChannelsPerFrame;
    uint32_t mBitsPerChannel;
    uint32_t mReserved;
};

struct IOSAudioQueueBuffer {
    uint32_t mAudioDataBytesCapacity;
    void* mAudioData;
    uint32_t mAudioDataByteSize;
    void* mUserData;
    uint32_t mPacketDescriptionCapacity;
    void* mPacketDescriptions;
    uint32_t mPacketDescriptionCount;
};

typedef void* IOSAudioQueueRef;
typedef IOSAudioQueueBuffer* IOSAudioQueueBufferRef;
typedef void (*IOSAudioQueueInputCallback)(void*, IOSAudioQueueRef, IOSAudioQueueBufferRef, const void*, uint32_t, const void*);
typedef void (*IOSAudioQueueOutputCallback)(void*, IOSAudioQueueRef, IOSAudioQueueBufferRef);

typedef int32_t (*PFN_AudioQueueNewInput)(const IOSAudioStreamBasicDescription*, IOSAudioQueueInputCallback, void*, void*, void*, uint32_t, IOSAudioQueueRef*);
typedef int32_t (*PFN_AudioQueueNewOutput)(const IOSAudioStreamBasicDescription*, IOSAudioQueueOutputCallback, void*, void*, void*, uint32_t, IOSAudioQueueRef*);
typedef int32_t (*PFN_AudioQueueAllocateBuffer)(IOSAudioQueueRef, uint32_t, IOSAudioQueueBufferRef*);
typedef int32_t (*PFN_AudioQueueEnqueueBuffer)(IOSAudioQueueRef, IOSAudioQueueBufferRef, uint32_t, const void*);
typedef int32_t (*PFN_AudioQueueStart)(IOSAudioQueueRef, const void*);
typedef int32_t (*PFN_AudioQueueStop)(IOSAudioQueueRef, uint8_t);
typedef int32_t (*PFN_AudioQueueDispose)(IOSAudioQueueRef, uint8_t);

static PFN_AudioQueueNewInput s_pfnAQNewInput = nullptr;
static PFN_AudioQueueNewOutput s_pfnAQNewOutput = nullptr;
static PFN_AudioQueueAllocateBuffer s_pfnAQAllocBuf = nullptr;
static PFN_AudioQueueEnqueueBuffer s_pfnAQEnqueueBuf = nullptr;
static PFN_AudioQueueStart s_pfnAQStart = nullptr;
static PFN_AudioQueueStop s_pfnAQStop = nullptr;
static PFN_AudioQueueDispose s_pfnAQDispose = nullptr;

static IOSAudioQueueRef s_iosInputQueue = nullptr;
static IOSAudioQueueRef s_iosOutputQueue = nullptr;

static bool LoadIOSAudioToolboxSymbols() {
    static bool s_bTried = false;
    if (s_bTried) return (s_pfnAQNewInput != nullptr);
    s_bTried = true;
    void* h = dlopen("/System/Library/Frameworks/AudioToolbox.framework/AudioToolbox", RTLD_LAZY);
    if (!h) h = RTLD_DEFAULT;
    s_pfnAQNewInput   = (PFN_AudioQueueNewInput)dlsym(h, "AudioQueueNewInput");
    s_pfnAQNewOutput  = (PFN_AudioQueueNewOutput)dlsym(h, "AudioQueueNewOutput");
    s_pfnAQAllocBuf   = (PFN_AudioQueueAllocateBuffer)dlsym(h, "AudioQueueAllocateBuffer");
    s_pfnAQEnqueueBuf = (PFN_AudioQueueEnqueueBuffer)dlsym(h, "AudioQueueEnqueueBuffer");
    s_pfnAQStart      = (PFN_AudioQueueStart)dlsym(h, "AudioQueueStart");
    s_pfnAQStop       = (PFN_AudioQueueStop)dlsym(h, "AudioQueueStop");
    s_pfnAQDispose    = (PFN_AudioQueueDispose)dlsym(h, "AudioQueueDispose");
    return (s_pfnAQNewInput && s_pfnAQNewOutput && s_pfnAQAllocBuf && s_pfnAQEnqueueBuf && s_pfnAQStart && s_pfnAQStop && s_pfnAQDispose);
}

static bool ConfigureIOSAudioSessionForVoice() {
    if (!EnsureObjcRuntimeLoaded()) return false;
    ObjcClass bundleCls = s_objc_getClass("NSBundle");
    ObjcClass strCls = s_objc_getClass("NSString");
    if (!bundleCls || !strCls) return false;
    ObjcId mainBundle = ((ObjcId(*)(ObjcId, ObjcSEL))s_objc_msgSend)((ObjcId)bundleCls, s_sel_registerName("mainBundle"));
    ObjcId keyStr = ((ObjcId(*)(ObjcId, ObjcSEL, const char*))s_objc_msgSend)((ObjcId)strCls, s_sel_registerName("stringWithUTF8String:"), "NSMicrophoneUsageDescription");
    ObjcId descObj = mainBundle ? ((ObjcId(*)(ObjcId, ObjcSEL, ObjcId))s_objc_msgSend)(mainBundle, s_sel_registerName("objectForInfoDictionaryKey:"), keyStr) : nullptr;
    if (!descObj) {
        LOGE("[VoiceChat] iOS Info.plist is missing NSMicrophoneUsageDescription! Add it to Info.plist to enable microphone.");
        return false;
    }

    ObjcClass sessCls = s_objc_getClass("AVAudioSession");
    if (!sessCls) return false;
    ObjcId session = ((ObjcId(*)(ObjcId, ObjcSEL))s_objc_msgSend)((ObjcId)sessCls, s_sel_registerName("sharedInstance"));
    if (!session) return false;

    ObjcId catStr = ((ObjcId(*)(ObjcId, ObjcSEL, const char*))s_objc_msgSend)((ObjcId)strCls, s_sel_registerName("stringWithUTF8String:"), "AVAudioSessionCategoryPlayAndRecord");
    ObjcSEL respSel = s_sel_registerName("respondsToSelector:");
    ObjcSEL setCatSel = s_sel_registerName("setCategory:withOptions:error:");
    if (((bool(*)(ObjcId, ObjcSEL, ObjcSEL))s_objc_msgSend)(session, respSel, setCatSel)) {
        ((bool(*)(ObjcId, ObjcSEL, ObjcId, unsigned long, ObjcId*))s_objc_msgSend)(session, setCatSel, catStr, 0xDu, nullptr);
    }
    ObjcSEL setActSel = s_sel_registerName("setActive:error:");
    if (((bool(*)(ObjcId, ObjcSEL, ObjcSEL))s_objc_msgSend)(session, respSel, setActSel)) {
        ((bool(*)(ObjcId, ObjcSEL, bool, ObjcId*))s_objc_msgSend)(session, setActSel, true, nullptr);
    }
    return true;
}

static void IOSVoiceInputCallback(void*, IOSAudioQueueRef inAQ, IOSAudioQueueBufferRef inBuffer, const void*, uint32_t, const void*) {
    if (!s_bMobileVoiceRecording) return;
    if (inBuffer && inBuffer->mAudioData && inBuffer->mAudioDataByteSize >= 2) {
        PushCapturedVoiceSamples((const int16_t*)inBuffer->mAudioData, inBuffer->mAudioDataByteSize / sizeof(int16_t));
    }
    if (s_bMobileVoiceRecording && s_pfnAQEnqueueBuf) {
        s_pfnAQEnqueueBuf(inAQ, inBuffer, 0, nullptr);
    }
}

static void IOSVoiceOutputCallback(void*, IOSAudioQueueRef outAQ, IOSAudioQueueBufferRef outBuffer) {
    if (!outBuffer || !outBuffer->mAudioData) return;
    std::vector<int16_t> chunk;
    pthread_mutex_lock(&s_voiceMutex);
    if (!s_playbackVoiceQueue.empty()) {
        chunk = std::move(s_playbackVoiceQueue.front());
        s_playbackVoiceQueue.pop_front();
    }
    pthread_mutex_unlock(&s_voiceMutex);

    if (!chunk.empty()) {
        uint32_t copyBytes = std::min<uint32_t>((uint32_t)(chunk.size() * sizeof(int16_t)), outBuffer->mAudioDataBytesCapacity);
        memcpy(outBuffer->mAudioData, chunk.data(), copyBytes);
        outBuffer->mAudioDataByteSize = copyBytes;
    } else {
        memset(outBuffer->mAudioData, 0, 1600 * sizeof(int16_t));
        outBuffer->mAudioDataByteSize = 1600 * sizeof(int16_t);
    }
    if (s_pfnAQEnqueueBuf) {
        s_pfnAQEnqueueBuf(outAQ, outBuffer, 0, nullptr);
    }
}
#endif // __APPLE__

extern "C" {

bool MobileVoiceChat_Init(uint32_t /*sampleRate*/, uint8_t /*channelCount*/) {
    if (s_bMobileVoiceInitialized) return true;
#if !defined(__APPLE__)
    s_bAndroidPlayThreadRunning = true;
    pthread_create(&s_androidPlayThread, nullptr, AndroidVoicePlayThreadProc, nullptr);
    pthread_detach(s_androidPlayThread);
#else
    if (LoadIOSAudioToolboxSymbols()) {
        IOSAudioStreamBasicDescription fmt{};
        fmt.mSampleRate = 32000.0;
        fmt.mFormatID = 0x6C70636Du; // 'lpcm'
        fmt.mFormatFlags = 0xCu;     // SignedInteger | Packed
        fmt.mBytesPerPacket = 2;
        fmt.mFramesPerPacket = 1;
        fmt.mBytesPerFrame = 2;
        fmt.mChannelsPerFrame = 1;
        fmt.mBitsPerChannel = 16;
        if (s_pfnAQNewOutput(&fmt, IOSVoiceOutputCallback, nullptr, nullptr, nullptr, 0, &s_iosOutputQueue) == 0 && s_iosOutputQueue) {
            for (int i = 0; i < 3; ++i) {
                IOSAudioQueueBufferRef buf = nullptr;
                if (s_pfnAQAllocBuf(s_iosOutputQueue, 3200 * sizeof(int16_t), &buf) == 0 && buf) {
                    memset(buf->mAudioData, 0, 1600 * sizeof(int16_t));
                    buf->mAudioDataByteSize = 1600 * sizeof(int16_t);
                    s_pfnAQEnqueueBuf(s_iosOutputQueue, buf, 0, nullptr);
                }
            }
            s_pfnAQStart(s_iosOutputQueue, nullptr);
        }
    }
#endif
    s_bMobileVoiceInitialized = true;
    return true;
}

void MobileVoiceChat_Destroy() {
    s_bMobileVoiceRecording = false;
#if !defined(__APPLE__)
    s_bAndroidPlayThreadRunning = false;
#else
    if (s_iosInputQueue && s_pfnAQStop && s_pfnAQDispose) {
        s_pfnAQStop(s_iosInputQueue, 1);
        s_pfnAQDispose(s_iosInputQueue, 1);
        s_iosInputQueue = nullptr;
    }
    if (s_iosOutputQueue && s_pfnAQStop && s_pfnAQDispose) {
        s_pfnAQStop(s_iosOutputQueue, 1);
        s_pfnAQDispose(s_iosOutputQueue, 1);
        s_iosOutputQueue = nullptr;
    }
#endif
    s_bMobileVoiceInitialized = false;
}

#ifdef __APPLE__
extern "C" void* _NSConcreteGlobalBlock[];
struct IOSPermBlockDescriptor {
    unsigned long reserved;
    unsigned long size;
};
struct IOSPermBlockLiteral {
    void* isa;
    int flags;
    int reserved;
    void (*invoke)(void*, bool);
    const IOSPermBlockDescriptor* descriptor;
};
static void OnIOSRecordPermBlockInvoke(void* /*block*/, bool granted) {
    s_bMobileVoicePermGranted = granted;
}
static const IOSPermBlockDescriptor s_iosPermBlockDesc = { 0, sizeof(IOSPermBlockLiteral) };
static IOSPermBlockLiteral s_iosPermBlock = {
    _NSConcreteGlobalBlock,
    (1 << 28) | (1 << 29), // BLOCK_IS_GLOBAL
    0,
    OnIOSRecordPermBlockInvoke,
    &s_iosPermBlockDesc
};
#endif

bool MobileVoiceChat_RequestPermission() {
#if !defined(__APPLE__)
    return CheckOrRequestAndroidMicPermission();
#else
    if (!LoadIOSAudioToolboxSymbols() || !ConfigureIOSAudioSessionForVoice()) return false;
    if (EnsureObjcRuntimeLoaded()) {
        if (ObjcClass sessCls = s_objc_getClass("AVAudioSession")) {
            ObjcId session = ((ObjcId(*)(ObjcId, ObjcSEL))s_objc_msgSend)((ObjcId)sessCls, s_sel_registerName("sharedInstance"));
            if (session) {
                ObjcSEL respSel = s_sel_registerName("respondsToSelector:");
                ObjcSEL reqPermSel = s_sel_registerName("requestRecordPermission:");
                if (((bool(*)(ObjcId, ObjcSEL, ObjcSEL))s_objc_msgSend)(session, respSel, reqPermSel)) {
                    ((void(*)(ObjcId, ObjcSEL, void*))s_objc_msgSend)(session, reqPermSel, (void*)&s_iosPermBlock);
                }
            }
        }
    }
    if (!s_bMobileVoicePermGranted && !s_iosInputQueue) {
        IOSAudioStreamBasicDescription fmt{};
        fmt.mSampleRate = 32000.0;
        fmt.mFormatID = 0x6C70636Du;
        fmt.mFormatFlags = 0xCu;
        fmt.mBytesPerPacket = 2;
        fmt.mFramesPerPacket = 1;
        fmt.mBytesPerFrame = 2;
        fmt.mChannelsPerFrame = 1;
        fmt.mBitsPerChannel = 16;
        IOSAudioQueueRef tmpQ = nullptr;
        if (s_pfnAQNewInput(&fmt, IOSVoiceInputCallback, nullptr, nullptr, nullptr, 0, &tmpQ) == 0 && tmpQ) {
            IOSAudioQueueBufferRef buf = nullptr;
            if (s_pfnAQAllocBuf(tmpQ, 1600 * sizeof(int16_t), &buf) == 0 && buf) {
                s_pfnAQEnqueueBuf(tmpQ, buf, 0, nullptr);
            }
            s_pfnAQStart(tmpQ, nullptr);
            s_pfnAQStop(tmpQ, 1);
            s_pfnAQDispose(tmpQ, 1);
            s_bMobileVoicePermGranted = true;
        }
    }
    return true;
#endif
}

bool MobileVoiceChat_StartRecording() {
    if (s_bMobileVoiceRecording) return true;
#if !defined(__APPLE__)
    if (!CheckOrRequestAndroidMicPermission()) return false;
    s_bMobileVoiceRecording = true;
    if (pthread_create(&s_androidMicThread, nullptr, AndroidMicCaptureThreadProc, nullptr) == 0) {
        pthread_detach(s_androidMicThread);
        return true;
    }
    s_bMobileVoiceRecording = false;
    return false;
#else
    if (!LoadIOSAudioToolboxSymbols() || !ConfigureIOSAudioSessionForVoice()) return false;
    IOSAudioStreamBasicDescription fmt{};
    fmt.mSampleRate = 32000.0;
    fmt.mFormatID = 0x6C70636Du; // 'lpcm'
    fmt.mFormatFlags = 0xCu;     // SignedInteger | Packed
    fmt.mBytesPerPacket = 2;
    fmt.mFramesPerPacket = 1;
    fmt.mBytesPerFrame = 2;
    fmt.mChannelsPerFrame = 1;
    fmt.mBitsPerChannel = 16;

    if (s_pfnAQNewInput(&fmt, IOSVoiceInputCallback, nullptr, nullptr, nullptr, 0, &s_iosInputQueue) != 0 || !s_iosInputQueue) {
        LOGE("[VoiceChat] iOS AudioQueueNewInput failed");
        return false;
    }
    s_bMobileVoiceRecording = true;
    for (int i = 0; i < 3; ++i) {
        IOSAudioQueueBufferRef buf = nullptr;
        if (s_pfnAQAllocBuf(s_iosInputQueue, 1600 * sizeof(int16_t), &buf) == 0 && buf) {
            s_pfnAQEnqueueBuf(s_iosInputQueue, buf, 0, nullptr);
        }
    }
    if (s_pfnAQStart(s_iosInputQueue, nullptr) != 0) {
        s_bMobileVoiceRecording = false;
        s_pfnAQDispose(s_iosInputQueue, 1);
        s_iosInputQueue = nullptr;
        return false;
    }
    return true;
#endif
}

void MobileVoiceChat_StopRecording() {
    if (!s_bMobileVoiceRecording) return;
    s_bMobileVoiceRecording = false;
#ifdef __APPLE__
    if (s_iosInputQueue && s_pfnAQStop && s_pfnAQDispose) {
        s_pfnAQStop(s_iosInputQueue, 1);
        s_pfnAQDispose(s_iosInputQueue, 1);
        s_iosInputQueue = nullptr;
    }
#endif
}

void MobileVoiceChat_AddPlaybackSamples(const int16_t* samples, size_t count, float volumePercent) {
    if (!samples || count == 0 || volumePercent <= 0.01f) return;
    float gain = volumePercent / 100.0f;
    std::vector<int16_t> scaled(count);
    for (size_t i = 0; i < count; ++i) {
        int32_t v = (int32_t)(samples[i] * gain);
        if (v > 32767) v = 32767;
        else if (v < -32768) v = -32768;
        scaled[i] = (int16_t)v;
    }
    pthread_mutex_lock(&s_voiceMutex);
    if (s_playbackVoiceQueue.size() < 40) {
        s_playbackVoiceQueue.push_back(std::move(scaled));
    }
    pthread_mutex_unlock(&s_voiceMutex);
}

void MobileVoiceChat_PollCapturedSamples(VoiceChat* pVoiceChat) {
    if (!pVoiceChat) return;
    while (true) {
        std::vector<int16_t> chunk;
        pthread_mutex_lock(&s_voiceMutex);
        if (!s_capturedVoiceQueue.empty()) {
            chunk = std::move(s_capturedVoiceQueue.front());
            s_capturedVoiceQueue.pop_front();
        }
        pthread_mutex_unlock(&s_voiceMutex);
        if (chunk.empty()) break;
        pVoiceChat->onCaptureSamples(chunk.data(), (uint32_t)chunk.size());
    }
}

#endif // ENABLE_VOICE_CHAT

bool AndroidDecodeImageFromMemory(const void* data, unsigned int size, unsigned int* outW, unsigned int* outH, std::vector<uint32_t>& outBGRA) {
    // Fast native decoder for 1-bit monochrome BMPs (such as CMonochromeCanvas 128x128 BotKiller Captcha)
    if (data && size >= 62 && outW && outH) {
        const uint8_t* b = static_cast<const uint8_t*>(data);
        if (b[0] == 'B' && b[1] == 'M') {
            uint32_t pixelOffset = 0;
            int32_t bmpW = 0, bmpH = 0;
            uint16_t planes = 0, bpp = 0;
            memcpy(&pixelOffset, b + 10, 4);
            memcpy(&bmpW, b + 18, 4);
            memcpy(&bmpH, b + 22, 4);
            memcpy(&planes, b + 26, 2);
            memcpy(&bpp, b + 28, 2);
            bool topDown = (bmpH < 0);
            int32_t absH = topDown ? -bmpH : bmpH;
            if (planes == 1 && bpp == 1 && bmpW > 0 && bmpW <= 1024 && absH > 0 && absH <= 1024 && pixelOffset <= size) {
                uint32_t c0 = 0xFF000000u, c1 = 0xFFFFFFFFu;
                if (pixelOffset >= 62) {
                    c0 = 0xFF000000u | (uint32_t(b[56]) << 16) | (uint32_t(b[55]) << 8) | uint32_t(b[54]);
                    c1 = 0xFF000000u | (uint32_t(b[60]) << 16) | (uint32_t(b[59]) << 8) | uint32_t(b[58]);
                }
                uint32_t rowStride = ((uint32_t(bmpW) + 31u) / 32u) * 4u;
                if (pixelOffset + rowStride * uint32_t(absH) <= size) {
                    *outW = (unsigned int)bmpW;
                    *outH = (unsigned int)absH;
                    outBGRA.resize((size_t)bmpW * (size_t)absH);
                    const uint8_t* pix = b + pixelOffset;
                    for (int32_t y = 0; y < absH; ++y) {
                        int32_t srcY = topDown ? y : (absH - 1 - y);
                        const uint8_t* row = pix + (size_t)srcY * rowStride;
                        uint32_t* dst = &outBGRA[(size_t)y * (size_t)bmpW];
                        for (int32_t x = 0; x < bmpW; ++x) {
                            uint8_t bit = (row[x >> 3] >> (7 - (x & 7))) & 1u;
                            dst[x] = bit ? c1 : c0;
                        }
                    }
                    return true;
                }
            }
        }
    }
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

LOGFONTA g_AndroidActiveLogFont = { -12, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 0, "Tahoma" };

bool AndroidRenderCharGlyph(int fontSize, unsigned short ch, int* outW, int* outH, int* outAdv, uint32_t* outPixels64) {
#ifndef __APPLE__
    if (!g_pJavaVM) return false;
#endif
    if (!outW || !outH || !outAdv || !outPixels64) return false;

    int pxSize = (fontSize < 0) ? -fontSize : (fontSize > 0 ? fontSize : 14);
    if (pxSize < 11) pxSize = 12;
    if (pxSize > 40) pxSize = 40;

    const bool bIsBold  = (g_AndroidActiveLogFont.lfWeight >= 600);
    const bool bIsArial = (strcasestr(g_AndroidActiveLogFont.lfFaceName, "Arial") != nullptr);
    const uint32_t styleMask = (bIsBold ? 0x8000u : 0u) | (bIsArial ? 0x4000u : 0u);

    struct SGlyphCacheEntry {
        int w, h, adv;
        std::vector<uint32_t> rows; // 64 * h pixels
    };
    static std::unordered_map<uint32_t, SGlyphCacheEntry> s_glyphCache;
    uint32_t key = (((uint32_t)pxSize | styleMask) << 16) | (uint32_t)ch;
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
    static jobject s_tfTahomaNorm = nullptr, s_tfTahomaBold = nullptr;
    static jobject s_tfArialNorm = nullptr, s_tfArialBold = nullptr;
    static jintArray s_pixArr = nullptr;
    static jmethodID s_setTextSize = nullptr, s_setFakeBoldText = nullptr, s_setTypeface = nullptr;
    static jmethodID s_measureText = nullptr, s_ascent = nullptr, s_descent = nullptr;
    static jmethodID s_drawText = nullptr, s_eraseColor = nullptr, s_getPixels = nullptr;
    static int s_lastPxSize = -1;
    static uint32_t s_lastStyleMask = 0xFFFFFFFFu;

    if (!s_paint) {
        jclass paintCls = env->FindClass("android/graphics/Paint");
        jmethodID paintCtor = env->GetMethodID(paintCls, "<init>", "(I)V");
        // Use ANTI_ALIAS_FLAG (1) ONLY — do NOT set SUBPIXEL_TEXT_FLAG (128) so FreeType stem hinting stays razor-sharp
        jobject p = env->NewObject(paintCls, paintCtor, 1);
        env->CallVoidMethod(p, env->GetMethodID(paintCls, "setColor", "(I)V"), (jint)0xFFFFFFFF);
        jmethodID setHintingMid = env->GetMethodID(paintCls, "setHinting", "(I)V");
        if (setHintingMid) env->CallVoidMethod(p, setHintingMid, (jint)1); // HINTING_ON
        env->ExceptionClear();

        s_paint = env->NewGlobalRef(p);
        s_setTextSize = env->GetMethodID(paintCls, "setTextSize", "(F)V");
        s_setFakeBoldText = env->GetMethodID(paintCls, "setFakeBoldText", "(Z)V");
        s_setTypeface = env->GetMethodID(paintCls, "setTypeface", "(Landroid/graphics/Typeface;)Landroid/graphics/Typeface;");
        s_measureText = env->GetMethodID(paintCls, "measureText", "(Ljava/lang/String;)F");
        s_ascent = env->GetMethodID(paintCls, "ascent", "()F");
        s_descent = env->GetMethodID(paintCls, "descent", "()F");

        jclass tfCls = env->FindClass("android/graphics/Typeface");
        if (tfCls) {
            jmethodID createMid = env->GetStaticMethodID(tfCls, "create", "(Ljava/lang/String;I)Landroid/graphics/Typeface;");
            if (createMid) {
                jstring jTahoma = env->NewStringUTF("sans-serif");
                jstring jArial  = env->NewStringUTF("sans-serif");
                jobject tfTN = env->CallStaticObjectMethod(tfCls, createMid, jTahoma, (jint)0);
                jobject tfTB = env->CallStaticObjectMethod(tfCls, createMid, jTahoma, (jint)1);
                jobject tfAN = env->CallStaticObjectMethod(tfCls, createMid, jArial,  (jint)0);
                jobject tfAB = env->CallStaticObjectMethod(tfCls, createMid, jArial,  (jint)1);
                if (tfTN) { s_tfTahomaNorm = env->NewGlobalRef(tfTN); env->DeleteLocalRef(tfTN); }
                if (tfTB) { s_tfTahomaBold = env->NewGlobalRef(tfTB); env->DeleteLocalRef(tfTB); }
                if (tfAN) { s_tfArialNorm  = env->NewGlobalRef(tfAN); env->DeleteLocalRef(tfAN); }
                if (tfAB) { s_tfArialBold  = env->NewGlobalRef(tfAB); env->DeleteLocalRef(tfAB); }
                env->DeleteLocalRef(jTahoma);
                env->DeleteLocalRef(jArial);
                env->ExceptionClear();
            }
            env->DeleteLocalRef(tfCls);
        }

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
    if (s_lastStyleMask != styleMask) {
        s_lastStyleMask = styleMask;
        if (s_setFakeBoldText)
            env->CallVoidMethod(s_paint, s_setFakeBoldText, (jboolean)(bIsBold ? JNI_TRUE : JNI_FALSE));
        jobject targetTf = bIsArial ? (bIsBold ? s_tfArialBold : s_tfArialNorm)
                                    : (bIsBold ? s_tfTahomaBold : s_tfTahomaNorm);
        if (s_setTypeface && targetTf) {
            jobject prevTf = env->CallObjectMethod(s_paint, s_setTypeface, targetTf);
            if (prevTf) env->DeleteLocalRef(prevTf);
        }
        env->ExceptionClear();
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
    env->CallVoidMethod(s_canvas, s_drawText, jstr, 0.0f, ceilf(-ascent), s_paint);
    env->CallVoidMethod(s_bmp, s_getPixels, s_pixArr, 0, 64, 0, 0, 64, h);
    env->GetIntArrayRegion(s_pixArr, 0, 64 * h, (jint*)outPixels64);

    env->DeleteLocalRef(jstr);
    env->ExceptionClear();
#endif

    // One-time stem contrast sharpening before caching in s_glyphCache (0 per-frame cost)
    const int totalGlyphPixels = 64 * h;
    for (int i = 0; i < totalGlyphPixels; ++i) {
        uint32_t px = outPixels64[i];
        uint32_t a = (px >> 24) & 0xFFu;
        if (a == 0) a = px & 0xFFu;
        if (a > 0) {
            if (a < 22u) {
                outPixels64[i] = 0;
            } else {
                uint32_t sharpA = (a >= 165u) ? 255u : ((a - 22u) * 255u) / 143u;
                outPixels64[i] = (sharpA << 24) | (sharpA << 16) | (sharpA << 8) | sharpA;
            }
        }
    }

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

#if !defined(__APPLE__)
#include <fcntl.h>
#endif

static void WriteCrashLine(int fd, const char* fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf) - 2, fmt, ap);
    va_end(ap);
    if (n > 0) {
        if (n > (int)sizeof(buf) - 2) n = (int)sizeof(buf) - 2;
        buf[n] = '\0';
        LOGE("%s", buf);
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
    WriteCrashLine(fd, "=== WORLD ONLINE NATIVE CRASH & MEMORY REPORT ===");
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
#elif defined(__aarch64__)
    ucontext_t* uc = (ucontext_t*)ucontext;
    if (uc) {
        char pcSym[256], lrSym[256];
        FormatAddressSymbol((void*)uc->uc_mcontext.pc, pcSym, sizeof(pcSym));
        FormatAddressSymbol((void*)uc->uc_mcontext.regs[30], lrSym, sizeof(lrSym));
        WriteCrashLine(fd, "--- ANDROID ARM64 CPU REGISTERS ---");
        WriteCrashLine(fd, "PC (Fault)  : %s", pcSym);
        WriteCrashLine(fd, "LR (Caller) : %s", lrSym);
        WriteCrashLine(fd, "SP=%p  FP=%p  FaultAddr=%p",
            (void*)uc->uc_mcontext.sp, (void*)uc->uc_mcontext.regs[29], (void*)uc->uc_mcontext.fault_address);
        WriteCrashLine(fd, "X0=%p X1=%p X2=%p X3=%p",
            (void*)uc->uc_mcontext.regs[0], (void*)uc->uc_mcontext.regs[1],
            (void*)uc->uc_mcontext.regs[2], (void*)uc->uc_mcontext.regs[3]);
    }
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
    signal(sig, SIG_DFL);
    raise(sig);
    _exit(1);
}

static void InstallAndroidCrashHandler() {
    signal(SIGPIPE, SIG_IGN);
    struct sigaction sa{};
    sa.sa_sigaction = AndroidCrashHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGBUS, &sa, nullptr);
    sigaction(SIGILL, &sa, nullptr);
    sigaction(SIGTRAP, &sa, nullptr);
    sigaction(SIGFPE, &sa, nullptr);
}
#endif // __APPLE__

#ifdef ENABLE_OPENGL
void initmobileInput(void);
#endif

static const char* GetMobileBaseDir() {
#ifdef __APPLE__
    return IOSGetDocumentsDirectory();
#else
    static std::string s_cachedBaseDir;
    if (!s_cachedBaseDir.empty())
        return s_cachedBaseDir.c_str();
    if (g_pJavaVM) {
        JNIEnv* env = nullptr;
        if (g_pJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_OK && env) {
            jclass atCls = env->FindClass("android/app/ActivityThread");
            if (atCls) {
                jmethodID curAppMid = env->GetStaticMethodID(atCls, "currentApplication", "()Landroid/app/Application;");
                jobject appObj = curAppMid ? env->CallStaticObjectMethod(atCls, curAppMid) : nullptr;
                env->ExceptionClear();
                if (appObj) {
                    jclass ctxCls = env->GetObjectClass(appObj);
                    jmethodID getExtMid = env->GetMethodID(ctxCls, "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;");
                    jobject fileObj = getExtMid ? env->CallObjectMethod(appObj, getExtMid, nullptr) : nullptr;
                    env->ExceptionClear();
                    if (fileObj) {
                        jclass fileCls = env->GetObjectClass(fileObj);
                        jmethodID getPathMid = env->GetMethodID(fileCls, "getAbsolutePath", "()Ljava/lang/String;");
                        jstring pathStr = getPathMid ? (jstring)env->CallObjectMethod(fileObj, getPathMid) : nullptr;
                        env->ExceptionClear();
                        if (pathStr) {
                            const char* utf = env->GetStringUTFChars(pathStr, nullptr);
                            if (utf && *utf) s_cachedBaseDir = utf;
                            if (utf) env->ReleaseStringUTFChars(pathStr, utf);
                            env->DeleteLocalRef(pathStr);
                        }
                        env->DeleteLocalRef(fileCls);
                        env->DeleteLocalRef(fileObj);
                    }
                    env->DeleteLocalRef(ctxCls);
                    env->DeleteLocalRef(appObj);
                }
                env->DeleteLocalRef(atCls);
            }
        }
    }
    if (!s_cachedBaseDir.empty())
        return s_cachedBaseDir.c_str();
    if (access("/sdcard/Android/data/com.worldonline.client/files", F_OK) == 0)
        return "/sdcard/Android/data/com.worldonline.client/files";
    return "/sdcard/Android/data/com.worldonline.client/files";
#endif
}

extern "C" uint32_t g_AndroidGLContextGen;
extern void CheckAndRestoreAndroidGLContext(IDirect3DDevice8* dev);
static pthread_mutex_t s_engineInitMutex = PTHREAD_MUTEX_INITIALIZER;

JNIEXPORT void JNICALL Java_com_worldonline_client_NativeLib_init(JNIEnv* env, jobject obj, jobject assetManager, jint width, jint height) {
    pthread_mutex_lock(&s_engineInitMutex);
    if (s_bEngineReady) {
        if (width > 0 && height > 0) {
            g_AndroidPhysWidth = (int)width;
            g_AndroidPhysHeight = (int)height;
            glViewport(0, 0, (GLsizei)width, (GLsizei)height);
        }
        if (IDirect3DDevice8* dev = GetCurrentGLDevice()) {
            CheckAndRestoreAndroidGLContext(dev);
            dev->m_pCurrentRenderTargetTex = NULL;
            for (int i = 0; i < 8; ++i) dev->m_BoundGLTexId[i] = 0xFFFFFFFFu;
            dev->m_DirtyFlags = 0xFFFFFFFFu;
        }
        LOGI("WorldOnline surface resized (%dx%d) — keeping active session (ctxGen=%u)", width, height, g_AndroidGLContextGen);
        pthread_mutex_unlock(&s_engineInitMutex);
        return;
    }

    LOGI("Initializing WorldOnline Engine (%dx%d)", width, height);
    srand((unsigned int)time(NULL) ^ (unsigned int)getpid());
    if (width > 0 && height > 0) {
        g_AndroidPhysWidth = (int)width;
        g_AndroidPhysHeight = (int)height;
        int landW = (width >= height) ? (int)width : (int)height;
        int landH = (width >= height) ? (int)height : (int)width;
        g_AndroidScreenHeight = 720;
        g_AndroidScreenWidth = (int)(((float)landW * 720.0f) / (float)landH + 0.5f);
        if (g_AndroidScreenWidth < 960) g_AndroidScreenWidth = 960;
    }
    s_bEngineReady = false;
    
#if !defined(__APPLE__)
    env->GetJavaVM(&g_pJavaVM);
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

    if (width > 0 && height > 0 && (access("worldonline.cfg", F_OK) != 0 || access("cfg_defaults_v4.flag", F_OK) != 0)) {
        unlink("worldonline.cfg");
        if (FILE* f = fopen("cfg_defaults_v4.flag", "wb")) fclose(f);
        if (access("worldonline.cfg", F_OK) != 0) {
            if (FILE* cfgFp = fopen("worldonline.cfg", "wb")) {
                fprintf(cfgFp,
                    "WIDTH\t\t\t\t\t\t%d\n"
                    "HEIGHT\t\t\t\t\t\t%d\n"
                    "BPP\t\t\t\t\t\t32\n"
                    "FREQUENCY\t\t\t\t\t60\n"
                    "SOFTWARE_CURSOR\t\t\t0\n"
                    "SOFTWARE_TILING\t\t\t0\n"
                    "OBJECT_CULLING\t\t\t\t1\n"
                    "VISIBILITY\t\t\t\t\t3\n"
                    "MUSIC_VOLUME\t\t\t\t0.100\n"
                    "VOICE_VOLUME\t\t\t\t5\n"
                    "GAMMA\t\t\t\t\t\t3\n"
                    "IS_SAVE_ID\t\t\t\t\t0\n"
                    "SAVE_ID\t\t\t\t\t0\n"
                    "DECOMPRESSED_TEXTURE\t\t0\n"
                    "WINDOWED\t\t\t\t\t1\n"
                    "FOG_MODE_ON\t\t\t\t1\n"
                    "NIGHT_MODE_ON\t\t\t0\n"
                    "SNOW_MODE_ON\t\t\t0\n"
                    "SNOW_TEXTURE_MODE\t\t0\n"
                    "RAIN_MODE_ON\t\t\t0\n"
                    "GRAPHICS_LEVEL\t\t\t\t0\n"
                    "SHADOW_QUALITY_LEVEL\t\t0\n"
                    "TERRAIN_SHADOW\t\t\t\t0\n"
                    "SHADOW_DETAILS\t\t\t\t0\n"
                    "SHADOW_LEVEL\t\t\t\t0\n"
                    "RAIN_LEVEL\t\t\t\t\t0\n",
                    (int)width, (int)height);
                fclose(cfgFp);
            }
        }
    }
    if (FILE* fp = fopen("lib/python2.7/config/Makefile", "a")) fclose(fp);
    if (FILE* fp = fopen("include/python2.7/pyconfig.h", "a")) fclose(fp);

    // 1. Open syserr.txt only (disable verbose log.txt for release)
    SetLogLevel(0);
    OpenLogFile(false);

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
    if (!PackInitialize("pack")) {
        LOGE("PackInitialize('pack') failed!");
        TraceError("PackInitialize('pack') failed!");
        pthread_mutex_unlock(&s_engineInitMutex);
        return;
    }
    CEterPackManager::Instance().SetSearchMode(true);

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
    s_pApp->Initialize(NULL);

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
            "def _wo_codec(name):\n"
            "    return (_codecs.latin_1_encode, _codecs.latin_1_decode, None, None)\n"
            "_codecs.register(_wo_codec)\n"
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
        initmobileInput();
#endif
        Tracen("worldonline Mobile: Running RunMainScript (system.py)...");
        if (!RunMainScript(s_pyLauncher, "")) {
            if (PyErr_Occurred()) {
                PyErr_Print();
            }
            fflush(stderr);
            LOGE("RunMainScript failed! Check syserr.txt");
            TraceError("RunMainScript failed!");
            pthread_mutex_unlock(&s_engineInitMutex);
            return;
        }
    } else {
        LOGE("s_pyLauncher.Create() failed!");
        TraceError("s_pyLauncher.Create() failed!");
        pthread_mutex_unlock(&s_engineInitMutex);
        return;
    }
	PyRun_SimpleString(
        "import __builtin__, sys, imp, weakref, time as _t\n"
        "_t.clock = getattr(_t, 'clock', _t.time)\n"
        "__builtin__._g_mobileScrollBars = []\n"
        "__builtin__._g_activeMobileScrollBar = None\n"
        "__builtin__._g_activeMobileScrollTrackH = 200.0\n"
        "def _MobileHookScrollBars():\n"
        "    try:\n"
        "        import ui\n"
        "        if getattr(ui, '_mobileScrollHooked', False): return\n"
        "        ui._mobileScrollHooked = True\n"
        "        for clsName in ('ScrollBar', 'ThinScrollBar', 'SmallThinScrollBar'):\n"
        "            cls = getattr(ui, clsName, None)\n"
        "            if cls is None: continue\n"
        "            origInit = cls.__init__\n"
        "            def _makeInit(oInit):\n"
        "                def _wrappedInit(self, *a, **k):\n"
        "                    oInit(self, *a, **k)\n"
        "                    try: __builtin__._g_mobileScrollBars.append(weakref.ref(self))\n"
        "                    except: pass\n"
        "                return _wrappedInit\n"
        "            cls.__init__ = _makeInit(origInit)\n"
        "    except:\n"
        "        pass\n"
        "__builtin__._MobileHookScrollBars = _MobileHookScrollBars\n"
        "def _MobileBeginTouchScroll(mx, my):\n"
        "    __builtin__._MobileHookScrollBars()\n"
        "    __builtin__._g_activeMobileScrollBar = None\n"
        "    alive = []\n"
        "    bestSb = None\n"
        "    bestArea = 999999999\n"
        "    for r in __builtin__._g_mobileScrollBars:\n"
        "        sb = r()\n"
        "        if sb is None: continue\n"
        "        alive.append(r)\n"
        "        try:\n"
        "            if not sb.IsShow(): continue\n"
        "            cur = sb\n"
        "            vis = True\n"
        "            topWnd = sb\n"
        "            while cur is not None:\n"
        "                if not cur.IsShow(): vis = False; break\n"
        "                p = cur.GetParentProxy() if hasattr(cur, 'GetParentProxy') else None\n"
        "                if p is None or p == cur: break\n"
        "                topWnd = p\n"
        "                cur = p\n"
        "            if not vis: continue\n"
        "            checkWnd = sb.GetParentProxy() if hasattr(sb, 'GetParentProxy') and sb.GetParentProxy() else topWnd\n"
        "            for cand in (checkWnd, topWnd):\n"
        "                if cand is None: continue\n"
        "                gx, gy = cand.GetGlobalPosition()\n"
        "                gw, gh = cand.GetWidth(), cand.GetHeight()\n"
        "                if gw <= 40 or gh <= 40: continue\n"
        "                if gx <= mx <= gx + gw and gy <= my <= gy + gh:\n"
        "                    # Treat outer edges of movable top window as draggable window borders\n"
        "                    tx, ty = topWnd.GetGlobalPosition()\n"
        "                    tw, th = topWnd.GetWidth(), topWnd.GetHeight()\n"
        "                    if tw < 900 and th < 680:\n"
        "                        if (my <= ty + 36) or (my >= ty + th - 18) or (mx <= tx + 22) or (mx >= tx + tw - 22):\n"
        "                            continue\n"
        "                    area = gw * gh\n"
        "                    if area < bestArea:\n"
        "                        bestArea = area\n"
        "                        bestSb = sb\n"
        "        except:\n"
        "            pass\n"
        "    __builtin__._g_mobileScrollBars = alive\n"
        "    if bestSb is not None:\n"
        "        __builtin__._g_activeMobileScrollBar = bestSb\n"
        "        try: __builtin__._g_activeMobileScrollTrackH = float(max(120, bestSb.GetHeight()))\n"
        "        except: __builtin__._g_activeMobileScrollTrackH = 220.0\n"
        "        return 1\n"
        "    return 0\n"
        "__builtin__._MobileBeginTouchScroll = _MobileBeginTouchScroll\n"
        "def _MobileUpdateTouchScroll(stepDy):\n"
        "    sb = __builtin__._g_activeMobileScrollBar\n"
        "    if sb is None: return 0\n"
        "    try:\n"
        "        if not sb.IsShow(): return 0\n"
        "        curPos = float(sb.GetPos())\n"
        "        trackH = float(getattr(__builtin__, '_g_activeMobileScrollTrackH', 220.0))\n"
        "        newPos = max(0.0, min(1.0, curPos - (float(stepDy) / (trackH * 0.85))))\n"
        "        sb.SetPos(newPos)\n"
        "        return 1\n"
        "    except:\n"
        "        return 0\n"
        "__builtin__._MobileUpdateTouchScroll = _MobileUpdateTouchScroll\n"
        "__builtin__._g_mobileGhostImg = None\n"
        "def _UpdateMobileAttachedGhost(rx, ry):\n"
        "    try:\n"
        "        import mouseModule, ui, item, player\n"
        "        mc = getattr(mouseModule, 'mouseController', None)\n"
        "        if mc is not None:\n"
        "            mc.Render = lambda *a: None\n"
        "            nl = getattr(mc, 'NumberLine', None)\n"
        "            if nl is not None and nl.IsShow():\n"
        "                nl.Hide()\n"
        "        if mc is None or not mc.isAttached():\n"
        "            if __builtin__._g_mobileGhostImg is not None and __builtin__._g_mobileGhostImg.IsShow():\n"
        "                __builtin__._g_mobileGhostImg.Hide()\n"
        "            return\n"
        "        vnum = int(mc.GetAttachedItemIndex())\n"
        "        cnt = int(mc.GetAttachedItemCount())\n"
        "        attType = int(mc.GetAttachedType())\n"
        "        if vnum <= 0 or attType != player.SLOT_TYPE_INVENTORY:\n"
        "            if __builtin__._g_mobileGhostImg is not None and __builtin__._g_mobileGhostImg.IsShow():\n"
        "                __builtin__._g_mobileGhostImg.Hide()\n"
        "            return\n"
        "        gImg = __builtin__._g_mobileGhostImg\n"
        "        if gImg is None:\n"
        "            gImg = ui.ExpandedImageBox('TOP_MOST')\n"
        "            gImg.AddFlag('not_pick')\n"
        "            gImg.AddFlag('float')\n"
        "            gImg._lastVnum = -1\n"
        "            cntTxt = ui.TextLine()\n"
        "            cntTxt.SetParent(gImg)\n"
        "            cntTxt.AddFlag('not_pick')\n"
        "            cntTxt.SetOutline()\n"
        "            gImg.cntTxt = cntTxt\n"
        "            __builtin__._g_mobileGhostImg = gImg\n"
        "        if getattr(gImg, '_lastVnum', -1) != vnum:\n"
        "            item.SelectItem(vnum)\n"
        "            gImg.LoadImage(item.GetIconImageFileName())\n"
        "            gImg._lastVnum = vnum\n"
        "        gw = max(32, gImg.GetWidth())\n"
        "        gh = max(32, gImg.GetHeight())\n"
        "        gImg.SetPosition(int(rx - gw / 2), int(ry - gh / 2))\n"
        "        if cnt > 1:\n"
        "            gImg.cntTxt.SetText(str(cnt))\n"
        "            gImg.cntTxt.SetPosition(gw - 14, gh - 14)\n"
        "            gImg.cntTxt.Show()\n"
        "        else:\n"
        "            gImg.cntTxt.Hide()\n"
        "        if not gImg.IsShow():\n"
        "            gImg.Show()\n"
        "        gImg.SetTop()\n"
        "    except:\n"
        "        pass\n"
        "__builtin__._UpdateMobileAttachedGhost = _UpdateMobileAttachedGhost\n"
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

    s_bEngineReady = true;
    Tracen("worldonline Mobile: Engine Initialized Successfully!");
    LOGI("worldonline Mobile Engine Initialized Successfully");
    pthread_mutex_unlock(&s_engineInitMutex);
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
    MICON_MAP_PIN = 11,
    MICON_EMOTE_SMILEY = 12,
    MICON_AUTO_COMBAT = 13,
    MICON_QUEST_SCROLL = 14
};

// Deterministic 2D hash for fine chiseled stone surface grain
static inline float StoneGrainNoise(int x, int y) {
    uint32_t n = (uint32_t)(x * 374761393u + y * 668265263u);
    n = (n ^ (n >> 13)) * 1274126177u;
    return ((float)(n & 0xFFu) / 255.0f) - 0.5f; // -0.5 .. +0.5
}

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

    // Razor-thin stone bezel (no thick golden tires)
    float thinRimW = fmaxf(1.6f, rimW * 0.38f);

    for (int y = 0; y < size; ++y) {
        float py = (y + 0.5f) - half;
        float ny = py / outerR;
        for (int x = 0; x < size; ++x) {
            float px = (x + 0.5f) - half;
            float nx = px / outerR;
            float d = hypotf(px, py);
            float grain = StoneGrainNoise(x, y);

            float r = 8.0f, g = 8.0f, b = 10.0f, a = 0.0f;

            // 1. Soft Outer Drop-Shadow
            if (d < outerR + 3.0f) {
                float sh = 1.0f - SmoothStepF(outerR - 0.5f, outerR + 3.0f, d);
                a = sh * (isPressed ? 165.0f : 125.0f);
            }

            float innerEdge = outerR - thinRimW;
            if (d <= outerR) {
                float bodyMask = 1.0f - SmoothStepF(outerR - 1.0f, outerR + 0.4f, d);
                float radRatio = fminf(1.0f, d / (innerEdge > 1.0f ? innerEdge : 1.0f));
                float sphereZ = sqrtf(fmaxf(0.0f, 1.0f - radRatio * radRatio));

                if (iconType == MICON_JOY_BG) {
                    // Classic Dark Translucent Joystick Base from image1.jpeg
                    float wellVig = 0.50f + 0.50f * (radRatio * radRatio);
                    float topCrescent = SmoothStepF(-0.18f, -0.85f, ny) * SmoothStepF(0.95f, 0.35f, radRatio) * 0.25f;
                    r = 16.0f * wellVig + topCrescent * 75.0f;
                    g = 16.0f * wellVig + topCrescent * 72.0f;
                    b = 18.0f * wellVig + topCrescent * 68.0f;
                    a = fmaxf(a, bodyMask * glassAlpha);
                } else {
                    // Classic Dark Smoky Slot & Joystick Knob Interior from image1.jpeg + 3D Top Glass Crescent Shade
                    float botGlow = SmoothStepF(-0.10f, 0.88f, ny) * (0.28f + 0.72f * powf(radRatio, 1.30f));
                    float boost = isPressed ? 1.40f : 1.0f;

                    r = (15.0f + botGlow * 24.0f) * boost;
                    g = (15.0f + botGlow * 22.0f) * boost;
                    b = (17.0f + botGlow * 20.0f) * boost;

                    float crescentEnv = hypotf(nx * 1.12f, (ny + 0.46f) * 1.75f);
                    float topCrescent = (1.0f - SmoothStepF(0.18f, 0.78f, crescentEnv)) * SmoothStepF(-0.05f, -0.78f, ny);
                    float hotSpot = powf(fmaxf(0.0f, -0.45f * nx - 0.65f * ny + sphereZ * 0.35f), 4.5f);
                    float glassShade = fminf(1.0f, topCrescent * 0.52f + hotSpot * 0.44f);

                    r = r * (1.0f - glassShade * 0.48f) + 238.0f * (glassShade * 0.48f);
                    g = g * (1.0f - glassShade * 0.48f) + 234.0f * (glassShade * 0.48f);
                    b = b * (1.0f - glassShade * 0.48f) + 225.0f * (glassShade * 0.48f);

                    float innerVig = SmoothStepF(0.76f, 0.99f, radRatio) * (1.0f - 0.5f * topCrescent);
                    r *= (1.0f - innerVig * 0.50f);
                    g *= (1.0f - innerVig * 0.50f);
                    b *= (1.0f - innerVig * 0.50f);

                    a = fmaxf(a, bodyMask * fminf(250.0f, glassAlpha + glassShade * 22.0f + (isPressed ? 18.0f : 0.0f)));
                }
            }

            // 2. Classic Warm Muted Bronze-Taupe Bezel Rim (matching image1.jpeg, including Joystick Knob)
            {
                float rimCenter = outerR - thinRimW * 0.5f;
                float rimDist = fabsf(d - rimCenter);
                if (rimDist < thinRimW * 0.90f) {
                    float rimMask = 1.0f - SmoothStepF(thinRimW * 0.25f, thinRimW * 0.90f, rimDist);
                    float lGrad = 0.58f + 0.42f * (-0.55f * nx - 0.83f * ny);
                    float specHighlight = powf(fmaxf(0.0f, -0.58f * nx - 0.81f * ny), 4.0f) * 36.0f;
                    float cr = fminf(255.0f, fmaxf(20.0f, (isPressed ? ar : rr) * lGrad + specHighlight));
                    float cg = fminf(255.0f, fmaxf(20.0f, (isPressed ? ag : rg) * lGrad + specHighlight * 0.92f));
                    float cb = fminf(255.0f, fmaxf(20.0f, (isPressed ? ab : rb) * lGrad + specHighlight * 0.78f));
                    r = r * (1.0f - rimMask) + cr * rimMask;
                    g = g * (1.0f - rimMask) + cg * rimMask;
                    b = b * (1.0f - rimMask) + cb * rimMask;
                    a = fmaxf(a, rimMask * 252.0f);
                }

                float grooveDist = fabsf(d - (innerEdge - 1.0f));
                if (grooveDist < 1.1f) {
                    float gm = (1.0f - grooveDist / 1.1f) * 0.70f;
                    r *= (1.0f - gm); g *= (1.0f - gm); b *= (1.0f - gm);
                }
            }

            // 3. Crisp Vector Icons (Warm Champagne-Ivory matching image1.jpeg)
            auto EvalIconDetail = [&](float ix, float iy, float& outTone) -> float {
                outTone = 1.0f;
                float id = hypotf(ix, iy);
                float m = 0.0f;
                if (iconType == MICON_JOY_BG) {
                    // Inner concentric ring + 4 Cardinal Directional Chevrons/Triangles from image1.jpeg
                    float recDist = fabsf(id - outerR * 0.44f);
                    if (recDist < 1.3f) m = (1.0f - recDist / 1.3f) * 0.32f;

                    float ax = fabsf(ix), ay = fabsf(iy);
                    float chevStart = outerR * 0.68f;
                    float chevTip   = outerR * 0.81f;
                    // Top & Bottom triangles
                    if (ay >= chevStart && ay <= chevTip) {
                        float t = ( chevTip - ay ) / (chevTip - chevStart);
                        float halfW = t * (outerR * 0.10f);
                        if (ax <= halfW + 0.8f)
                            m = fmaxf(m, (1.0f - SmoothStepF(fmaxf(0.0f, halfW - 0.6f), halfW + 0.8f, ax)) * 0.62f);
                    }
                    // Left & Right triangles
                    if (ax >= chevStart && ax <= chevTip) {
                        float t = ( chevTip - ax ) / (chevTip - chevStart);
                        float halfW = t * (outerR * 0.10f);
                        if (ay <= halfW + 0.8f)
                            m = fmaxf(m, (1.0f - SmoothStepF(fmaxf(0.0f, halfW - 0.6f), halfW + 0.8f, ay)) * 0.62f);
                    }
                } else if (iconType == MICON_JOY_KNOB) {
                    // Inner concentric ring + center dot on the dark translucent knob (matching image1.jpeg)
                    float cRing = fabsf(id - outerR * 0.54f);
                    if (cRing < 1.5f) m = (1.0f - cRing / 1.5f) * 0.52f;
                    if (id < 3.2f) m = fmaxf(m, (1.0f - SmoothStepF(1.2f, 3.2f, id)) * 0.35f);
                } else if (iconType == MICON_ATK_SWORDS) {
                    float s = outerR / 56.0f;
                    float ax = fabsf(ix), ay = fabsf(iy);
                    float rimInner = (outerR - thinRimW) / s;

                    if (ay >= (rimInner - 6.8f) * s && ay <= (rimInner + 0.4f) * s) {
                        float t = ((rimInner + 0.4f) * s - ay) / (7.2f * s);
                        float halfW = (1.0f - t) * 3.8f * s;
                        if (ax <= halfW + 0.7f * s) {
                            m = fmaxf(m, 1.0f - SmoothStepF(fmaxf(0.0f, halfW - 0.5f * s), halfW + 0.7f * s, ax));
                            outTone = (ix * iy >= 0.0f) ? 1.18f : 0.82f;
                        }
                    }
                    if (ax >= (rimInner - 6.8f) * s && ax <= (rimInner + 0.4f) * s) {
                        float t = ((rimInner + 0.4f) * s - ax) / (7.2f * s);
                        float halfW = (1.0f - t) * 3.8f * s;
                        if (ay <= halfW + 0.7f * s) {
                            m = fmaxf(m, 1.0f - SmoothStepF(fmaxf(0.0f, halfW - 0.5f * s), halfW + 0.7f * s, ay));
                            outTone = (ix * iy >= 0.0f) ? 1.18f : 0.82f;
                        }
                    }

                    float runeRing = fabsf(id - 41.5f * s);
                    if (runeRing < 1.3f * s) {
                        m = fmaxf(m, (1.0f - runeRing / (1.3f * s)) * 0.40f);
                    }

                    auto EvalSword = [&](float sx, float sy, float& sTone) -> float {
                        float sm = 0.0f;
                        sTone = 1.0f;
                        if (sx >= -6.0f * s && sx <= 27.5f * s) {
                            float t = (sx + 6.0f * s) / (33.5f * s);
                            float halfW = (t < 0.78f)
                                ? (3.5f * s + t * 0.65f * s)
                                : ((1.0f - (t - 0.78f) / 0.22f) * 4.15f * s);
                            float absV = fabsf(sy);
                            if (absV <= halfW + 0.9f * s) {
                                sm = 1.0f - SmoothStepF(fmaxf(0.0f, halfW - 0.8f * s), halfW + 0.9f * s, absV);
                                if (t > 0.06f && t < 0.72f && absV < 0.85f * s)
                                    sTone = 0.42f;
                                else
                                    sTone = (sy < 0.0f) ? 1.18f : 0.84f;
                            }
                        }
                        float guardCurve = -7.2f * s + (sy * sy) / (42.0f * s);
                        if (fabsf(sy) <= 13.5f * s) {
                            float gThick = (2.4f - 0.08f * (fabsf(sy) / s)) * s;
                            float gDist = fabsf(sx - guardCurve);
                            if (gDist <= gThick + 0.8f * s) {
                                float gm = 1.0f - SmoothStepF(fmaxf(0.0f, gThick - 0.6f * s), gThick + 0.8f * s, gDist);
                                if (gm > sm) { sm = gm; sTone = 1.10f; }
                            }
                        }
                        if (sx >= -18.5f * s && sx <= -7.5f * s && fabsf(sy) <= 2.2f * s) {
                            float hm = 1.0f - SmoothStepF(1.4f * s, 2.2f * s, fabsf(sy));
                            if (hm > sm) { sm = hm; sTone = 0.68f; }
                        }
                        float pomD = hypotf(sx + 21.5f * s, sy);
                        if (pomD <= 4.0f * s) {
                            float pm = 1.0f - SmoothStepF(2.7f * s, 4.0f * s, pomD);
                            if (pm > sm) { sm = pm; sTone = 1.12f; }
                        }
                        return sm;
                    };

                    float s1u = (ix - iy) * 0.70710678f, s1v = (ix + iy) * 0.70710678f;
                    float s2u = (-ix - iy) * 0.70710678f, s2v = (ix - iy) * 0.70710678f;
                    float t1 = 1.0f, t2 = 1.0f;
                    float sw1 = EvalSword(s1u, s1v, t1);
                    float sw2 = EvalSword(s2u, s2v, t2);
                    if (sw1 > 0.08f) {
                        if (sw1 >= m) { m = sw1; outTone = t1; }
                    } else if (sw2 > 0.08f) {
                        float underShadow = (fabsf(s1u) < 5.2f * s && fabsf(s1v) < 5.6f * s) ? 0.48f : 1.0f;
                        if (sw2 * underShadow >= m) { m = sw2 * underShadow; outTone = t2 * underShadow; }
                    }
                } else if (iconType == MICON_MOUNT) {
                    float ux = ix / (outerR / 28.0f);
                    float uy = iy / (outerR / 28.0f);
                    float hDist = 100.0f;
                    hDist = fminf(hDist, DistToSegF(ux, uy, -6.5f, 11.2f,  4.5f, 11.2f) - 2.0f);
                    hDist = fminf(hDist, DistToSegF(ux, uy, -3.2f,  9.5f, -4.2f, -1.5f) - 4.4f);
                    hDist = fminf(hDist, DistToSegF(ux, uy, -4.2f, -1.5f, -1.5f, -7.5f) - 4.0f);
                    hDist = fminf(hDist, hypotf(ux + 1.2f, uy + 6.5f) - 4.5f);
                    hDist = fminf(hDist, DistToSegF(ux, uy,  0.0f, -6.8f,  9.2f, -2.5f) - 3.1f);
                    hDist = fminf(hDist, DistToSegF(ux, uy, -1.0f, -3.8f,  6.5f, -1.5f) - 2.8f);
                    hDist = fminf(hDist, DistToSegF(ux, uy, -2.2f, -9.0f, -0.5f,-13.8f) - 1.35f);
                    hDist = fminf(hDist, DistToSegF(ux, uy, -4.2f, -8.2f, -3.5f,-12.8f) - 1.20f);
                    hDist = fminf(hDist, DistToSegF(ux, uy, -6.2f, -6.5f, -8.5f,  2.5f) - 2.2f);
                    hDist = fminf(hDist, DistToSegF(ux, uy, -8.2f,  1.5f, -7.2f,  8.5f) - 2.2f);
                    if (hDist < 0.8f) {
                        m = 1.0f - SmoothStepF(-0.5f, 0.8f, hDist);
                        if (hypotf(ux - 1.2f, uy + 6.8f) < 1.15f || hypotf(ux - 7.8f, uy + 2.6f) < 0.85f)
                            m *= 0.15f;
                    }
                } else if (iconType == MICON_LOOT) {
                    float ux = ix / (outerR / 28.0f);
                    float uy = iy / (outerR / 28.0f);
                    float palmD = hypotf(ux * 0.92f, (uy - 4.2f) * 1.05f) - 6.2f;
                    float wristBox = fmaxf(fabsf(ux) - 4.2f, fabsf(uy - 8.5f) - 2.8f);
                    float hDist = fminf(palmD, wristBox);
                    hDist = fminf(hDist, DistToSegF(ux, uy, -4.5f,  4.5f, -10.5f, -1.5f) - 1.9f);
                    hDist = fminf(hDist, DistToSegF(ux, uy, -4.2f,  0.5f,  -5.4f, -9.5f) - 1.65f);
                    hDist = fminf(hDist, DistToSegF(ux, uy, -1.4f, -0.5f,  -1.6f,-11.8f) - 1.70f);
                    hDist = fminf(hDist, DistToSegF(ux, uy,  1.6f,  0.0f,   2.2f,-10.5f) - 1.65f);
                    hDist = fminf(hDist, DistToSegF(ux, uy,  4.3f,  1.5f,   5.6f, -7.2f) - 1.50f);
                    if (hDist < 0.8f) {
                        m = 1.0f - SmoothStepF(-0.5f, 0.8f, hDist);
                        float crease = DistToSegF(ux, uy, -3.5f, 2.2f, 3.8f, 3.2f);
                        if (crease < 0.7f) outTone = 0.55f;
                    }
                } else if (iconType == MICON_TARGET) {
                    float s = outerR / 28.0f;
                    float rRing = fabsf(id - 11.0f * s);
                    if (rRing < 1.8f * s) m = 1.0f - SmoothStepF(0.6f * s, 1.8f * s, rRing);
                    if ((fabsf(ix) < 1.5f * s && fabsf(iy) >= 6.5f * s && fabsf(iy) <= 15.5f * s) ||
                        (fabsf(iy) < 1.5f * s && fabsf(ix) >= 6.5f * s && fabsf(ix) <= 15.5f * s))
                        m = fmaxf(m, 0.94f);
                    if (id < 2.5f * s)
                        m = fmaxf(m, 1.0f - SmoothStepF(1.0f * s, 2.5f * s, id));
                } else if (iconType == MICON_PAGE) {
                    float s = outerR / 28.0f;
                    float orbD = fabsf(id - 13.0f * s);
                    if (orbD < 1.8f * s && fabsf(ix) > 2.8f * s)
                        m = 1.0f - SmoothStepF(0.7f * s, 1.8f * s, orbD);
                } else if (iconType == MICON_CHAT_BUBBLE) {
                    float s = outerR / 28.0f;
                    float bd = hypotf(ix / (12.2f * s), (iy + 1.8f * s) / (9.6f * s));
                    float tail = DistToSegF(ix, iy, -4.5f * s, 5.0f * s, -8.8f * s, 9.8f * s) - 2.2f * s;
                    if (bd <= 1.0f || tail <= 0.0f) {
                        m = (bd <= 1.0f) ? (1.0f - SmoothStepF(0.84f, 1.0f, bd)) : (1.0f - SmoothStepF(-1.0f * s, 0.5f * s, tail));
                        if (fabsf(iy + 1.8f * s) < 1.5f * s && (fabsf(ix) < 1.5f * s || fabsf(fabsf(ix) - 5.4f * s) < 1.5f * s))
                            m = 0.10f;
                    }
                } else if (iconType == MICON_EMOTE_SMILEY) {
                    float s = outerR / 28.0f;
                    float faceR = 13.2f * s;
                    if (id <= faceR + 0.9f * s) {
                        m = 1.0f - SmoothStepF(faceR - 0.8f * s, faceR + 0.9f * s, id);
                        float eyeL = hypotf(ix + 4.5f * s, iy + 3.2f * s);
                        float eyeR = hypotf(ix - 4.5f * s, iy + 3.2f * s);
                        if (eyeL < 2.2f * s || eyeR < 2.2f * s)
                            m *= 0.08f;
                        float smileR = hypotf(ix, iy - 0.8f * s);
                        if (iy > 1.0f * s && smileR <= 8.2f * s && smileR >= 3.0f * s)
                            m *= 0.08f;
                    }
                } else if (iconType == MICON_AUTO_COMBAT) {
                    float s = outerR / 28.0f;
                    float d1 = DistToSegF(ix, iy, -9.8f * s, 9.8f * s, 9.8f * s, -9.8f * s);
                    float d2 = DistToSegF(ix, iy, 9.8f * s, 9.8f * s, -9.8f * s, -9.8f * s);
                    float g1 = DistToSegF(ix, iy, -8.5f * s, 2.5f * s, -2.5f * s, 8.5f * s);
                    float g2 = DistToSegF(ix, iy, 2.5f * s, 8.5f * s, 8.5f * s, 2.5f * s);
                    float ringA = (fabsf(id - 13.5f * s) < 1.4f * s && fabsf(ix) > 4.0f * s) ? 0.85f : 0.0f;
                    float md = fminf(fminf(d1, d2), fminf(g1, g2));
                    if (md < 2.0f * s)
                        m = 1.0f - SmoothStepF(0.8f * s, 2.0f * s, md);
                    m = fmaxf(m, ringA);
                } else if (iconType == MICON_QUEST_SCROLL) {
                    float s = outerR / 28.0f;
                    float ax = fabsf(ix), ay = fabsf(iy);
                    bool inBody = (ax <= 8.2f * s && ay <= 10.2f * s);
                    bool inRod = (ax <= 10.2f * s && fabsf(ay - 9.6f * s) <= 1.6f * s);
                    if (inBody || inRod) {
                        m = 0.96f;
                        float chk1 = DistToSegF(ix, iy, -3.6f * s, 0.5f * s, -0.8f * s, 3.6f * s);
                        float chk2 = DistToSegF(ix, iy, -0.8f * s, 3.6f * s, 4.6f * s, -3.2f * s);
                        if (fminf(chk1, chk2) < 1.8f * s)
                            m = 0.08f;
                    }
                } else if (iconType == MICON_PLUS) {
                    float s = outerR / 14.0f;
                    if ((fabsf(ix) < 1.45f * s && fabsf(iy) < 6.2f * s) || (fabsf(iy) < 1.45f * s && fabsf(ix) < 6.2f * s))
                        m = 0.96f;
                } else if (iconType == MICON_MINUS) {
                    float s = outerR / 14.0f;
                    if (fabsf(iy) < 1.45f * s && fabsf(ix) < 6.2f * s)
                        m = 0.96f;
                } else if (iconType == MICON_MAP_PIN) {
                    float s = outerR / 22.0f;
                    float ringD = fabsf(hypotf(ix, iy + 3.0f * s) - 5.5f * s);
                    if (ringD < 1.8f * s) m = 1.0f - SmoothStepF(0.6f * s, 1.8f * s, ringD);
                    float tailD = DistToSegF(ix, iy, 0.0f, 3.0f * s, 0.0f, 9.5f * s);
                    if (tailD < 2.2f * s) m = fmaxf(m, 1.0f - SmoothStepF(0.8f * s, 2.2f * s, tailD));
                }
                return m;
            };

            if (iconType != MICON_NONE) {
                float dummyTone = 1.0f;
                float shadowMask = (iconType >= MICON_ATK_SWORDS) ? EvalIconDetail(px - 1.5f, py - 1.8f, dummyTone) : 0.0f;
                if (shadowMask > 0.05f) {
                    float sm = shadowMask * 0.75f;
                    r *= (1.0f - sm); g *= (1.0f - sm); b *= (1.0f - sm);
                }

                float iconTone = 1.0f;
                float iconMask = EvalIconDetail(px, py, iconTone);
                if (iconMask > 0.0f) {
                    float vGrad = 0.5f - 0.5f * ny;
                    // Classic Warm Champagne-Taupe Icon Palette from image1.jpeg
                    float icR = fminf(255.0f, (isPressed ? 255.0f : (192.0f + vGrad * 46.0f)) * iconTone);
                    float icG = fminf(255.0f, (isPressed ? 238.0f : (178.0f + vGrad * 44.0f)) * iconTone);
                    float icB = fminf(255.0f, (isPressed ? 198.0f : (152.0f + vGrad * 38.0f)) * iconTone);
                    r = r * (1.0f - iconMask) + icR * iconMask;
                    g = g * (1.0f - iconMask) + icG * iconMask;
                    b = b * (1.0f - iconMask) + icB * iconMask;
                    a = fmaxf(a, iconMask * 254.0f);
                }
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
    const bool isClassicMask = (access("mark/minimap_classic.flag", F_OK) == 0);
    if (access("mark/m_minimap_mask.tga", F_OK) != 0) {
        FILE* fp = fopen("mark/m_minimap_mask.tga", "wb");
        if (fp) {
            uint8_t hdr[18] = {0};
            hdr[2] = 2; hdr[12] = 128; hdr[14] = 128; hdr[16] = 32; hdr[17] = 0x28;
            fwrite(hdr, 1, 18, fp);
            std::vector<uint8_t> pix(128 * 128 * 4, 255);
            for (int y = 0; y < 128; ++y) {
                for (int x = 0; x < 128; ++x) {
                    uint8_t a = 255;
                    if (isClassicMask) {
                        float dx = (x + 0.5f) - 64.0f;
                        float dy = (y + 0.5f) - 64.0f;
                        float d = hypotf(dx, dy);
                        float mask = 1.0f - SmoothStepF(57.0f, 60.0f, d);
                        a = ClampU8(mask * 255.0f);
                    } else {
                        int edgeDist = std::min(std::min(x, 127 - x), std::min(y, 127 - y));
                        a = (edgeDist <= 1) ? 0 : (edgeDist == 2 ? 180 : 255);
                    }
                    size_t i = ((size_t)y * 128 + (size_t)x) * 4;
                    pix[i + 0] = a; pix[i + 1] = a; pix[i + 2] = a; pix[i + 3] = a;
                }
            }
            fwrite(pix.data(), 1, pix.size(), fp);
            fclose(fp);
        }
    }

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
                    int dx = std::min(x, sz - 1 - x);
                    int dy = std::min(y, sz - 1 - y);
                    int d = std::min(dx, dy);
                    size_t i = ((size_t)y * sz + (size_t)x) * 4;
                    float grad = 0.72f + 0.28f * (1.0f - (float)(x + y) / (2.0f * sz));

                    if (d >= 1 && d <= 4) {
                        if (d == 1 || d == 4) {
                            pix[i+0] = 18; pix[i+1] = 20; pix[i+2] = 22; pix[i+3] = 245;
                        } else {
                            pix[i+0] = ClampU8(118.0f * grad);
                            pix[i+1] = ClampU8(142.0f * grad);
                            pix[i+2] = ClampU8(162.0f * grad);
                            pix[i+3] = 252;
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
    TICON_SYS = 10,
    TICON_SPEC_BOXES = 11
};

// Reverted to the clean white/silver vector top-bar design from image1.jpeg
// (keeping TICON_SHOP market stall and TICON_UTIL 4-tile star design!)
static void WriteTopBarButtonTGA(const char* path, int iconType, bool isPressed) {
    if (access(path, F_OK) == 0)
        return;

    const int size = 104;
    FILE* fp = fopen(path, "wb");
    if (!fp) return;

    uint8_t hdr[18] = {0};
    hdr[2] = 2; hdr[12] = size; hdr[14] = size; hdr[16] = 32; hdr[17] = 0x28;
    fwrite(hdr, 1, 18, fp);

    std::vector<uint8_t> pix((size_t)size * (size_t)size * 4, 0);
    float half = size * 0.5f;
    float s = size / 34.0f; // Classic clean proportion from image1.jpeg
    size_t idx = 0;

    auto EvalTopGlyph = [&](float gx, float gy) -> float {
        float ax = fabsf(gx), ay = fabsf(gy);
        float gd = hypotf(gx, gy);
        float m = 0.0f;

        if (iconType == TICON_COLLAPSE) {
            float d1 = DistToSegF(gx, gy, -3.5f, -5.8f,  4.2f, 0.0f);
            float d2 = DistToSegF(gx, gy,  4.2f,  0.0f, -3.5f, 5.8f);
            float md = fminf(d1, d2);
            if (md < 1.85f) m = 1.0f - SmoothStepF(0.8f, 1.85f, md);
        } else if (iconType == TICON_EXPAND) {
            float d1 = DistToSegF(gx, gy,  3.5f, -5.8f, -4.2f, 0.0f);
            float d2 = DistToSegF(gx, gy, -4.2f,  0.0f,  3.5f, 5.8f);
            float md = fminf(d1, d2);
            if (md < 1.85f) m = 1.0f - SmoothStepF(0.8f, 1.85f, md);
        } else if (iconType == TICON_SYS) {
            // Classic 3 Horizontal Bars from image1.jpeg
            for (int bIdx = -1; bIdx <= 1; ++bIdx) {
                float by = bIdx * 5.2f;
                float barD = DistToSegF(gx, gy, -6.8f, by, 6.8f, by);
                if (barD < 1.70f)
                    m = fmaxf(m, 1.0f - SmoothStepF(0.85f, 1.70f, barD));
            }
        } else if (iconType == TICON_CHAR) {
            // Classic Round Helmet Medallion with T-Visor Cutout from image1.jpeg
            if (gd <= 7.6f) {
                bool inEyeSlit = (gy >= -2.2f && gy <= -0.3f && ax <= 4.8f);
                bool inNasalCut = (gy >= -0.3f && gy <= 6.2f && ax <= 1.55f);
                if (!inEyeSlit && !inNasalCut)
                    m = 1.0f - SmoothStepF(6.7f, 7.6f, gd);
            }
        } else if (iconType == TICON_INVEN) {
            // Classic Chest Box with Horizontal Lid Line & Center Clasp from image1.jpeg
            if (ax <= 7.2f && gy >= -5.2f && gy <= 6.2f) {
                float edgeBox = fmaxf(ax - 7.2f, fmaxf(-5.2f - gy, gy - 6.2f));
                m = 1.0f - SmoothStepF(-0.7f, 0.2f, edgeBox);
                if (fabsf(gy + 0.6f) < 0.85f && ax > 1.8f)
                    m = 0.0f;
                else if (ax < 0.9f && fabsf(gy + 0.6f) < 0.9f)
                    m = 0.0f;
            }
        } else if (iconType == TICON_SPEC_BOXES) {
            // Classic Twin Offset Storage Boxes from image1.jpeg
            float b1x = gx + 2.6f, b1y = gy + 2.0f;
            float b2x = gx - 2.4f, b2y = gy - 2.0f;
            bool inFront = (fabsf(b2x) <= 5.2f && fabsf(b2y) <= 4.4f);
            bool inBack  = (fabsf(b1x) <= 5.0f && fabsf(b1y) <= 4.2f) && !(fabsf(b2x) <= 6.2f && fabsf(b2y) <= 5.4f);
            if (inFront) {
                m = 0.98f;
                if (fabsf(b2y + 0.6f) < 0.75f && fabsf(b2x) > 1.3f) m = 0.0f;
            } else if (inBack) {
                m = 0.92f;
                if (fabsf(b1y + 0.6f) < 0.75f) m = 0.0f;
            }
        } else if (iconType == TICON_COMM) {
            // Classic Two-Person Community Silhouette from image1.jpeg
            float fgHead = hypotf(gx + 2.0f, gy + 3.6f) - 2.8f;
            float fgBody = (gy >= 0.8f && gy <= 7.2f) ? (hypotf((gx + 2.0f) * 0.72f, gy - 5.2f) - 4.5f) : 10.0f;
            float fgD = fminf(fgHead, fgBody);

            float bgHead = hypotf(gx - 3.6f, gy + 4.4f) - 2.3f;
            float bgBody = (gy >= 0.0f && gy <= 6.0f) ? (hypotf((gx - 3.6f) * 0.78f, gy - 4.0f) - 3.6f) : 10.0f;
            float bgD = fminf(bgHead, bgBody);

            if (fgD < 0.5f) m = 1.0f - SmoothStepF(-0.5f, 0.5f, fgD);
            else if (bgD < 0.5f && fgD > 1.3f) m = 1.0f - SmoothStepF(-0.5f, 0.5f, bgD);
        } else if (iconType == TICON_DS) {
            // Classic Double-Diamond Alchemy Icon from image1.jpeg
            float dia = ax * 1.12f + ay * 0.86f;
            if (dia <= 8.2f) {
                if (dia >= 5.6f || dia <= 3.2f)
                    m = 1.0f - SmoothStepF(7.4f, 8.2f, dia);
            }
        } else if (iconType == TICON_SHOP) {
            // Kept New Open IkarusShop Market Stall Design
            if (gy >= -7.2f && gy <= -1.2f) {
                float t = (gy + 7.2f) / 6.0f;
                float halfW = 5.0f + t * 3.4f;
                if (ax <= halfW) {
                    float scallop = (gy > -2.3f) ? (0.42f * cosf(gx * 1.45f)) : 0.0f;
                    if (gy <= -1.5f + scallop) {
                        m = 0.98f;
                        if (fabsf(gx) < 0.42f || fabsf(ax - 2.8f) < 0.42f || fabsf(ax - 5.6f) < 0.42f)
                            m = 0.15f;
                    }
                }
            }
            if (gy >= -1.3f && gy <= 3.6f && ax >= 5.3f && ax <= 6.9f)
                m = fmaxf(m, 0.96f);
            float coinD = hypotf(gx, gy - 1.1f);
            if (coinD <= 2.2f && coinD >= 0.85f)
                m = fmaxf(m, 1.0f - SmoothStepF(1.6f, 2.2f, coinD));
            if (gy >= 3.5f && gy <= 4.9f && ax <= 8.2f)
                m = fmaxf(m, 0.98f);
            else if (gy > 4.9f && gy <= 7.2f && ax <= 7.4f) {
                if (!(fabsf(gy - 6.0f) < 0.45f && ax < 5.2f))
                    m = fmaxf(m, 0.98f);
            }
        } else if (iconType == TICON_UTIL) {
            // Kept New Utility 4-Tile Grid + Center Star Design (instead of old two swords)
            float tx = fabsf(ax - 4.3f);
            float ty = fabsf(ay - 4.3f);
            float tileBox = fmaxf(tx, ty);
            float centerStar = ax * 0.9f + ay * 0.9f;
            if (centerStar <= 2.5f) {
                m = 1.0f - SmoothStepF(1.8f, 2.5f, centerStar);
            } else if (ax >= 1.4f && ay >= 1.4f && tileBox <= 2.6f && tileBox >= 1.05f) {
                m = 1.0f - SmoothStepF(2.0f, 2.6f, tileBox);
            }
        }
        return m;
    };

    for (int y = 0; y < size; ++y) {
        float py = ((y + 0.5f) - half) / s;
        for (int x = 0; x < size; ++x) {
            float px = ((x + 0.5f) - half) / s;

            float r = 10.0f, g = 12.0f, b = 15.0f, a = 0.0f;

            // Subtle 1px dark drop-shadow contour (no dark circular halo!)
            float sh = fmaxf(EvalTopGlyph(px - 0.55f, py - 0.70f),
                             EvalTopGlyph(px + 0.35f, py + 0.45f));
            if (sh > 0.05f) {
                a = fmaxf(a, sh * 195.0f);
            }

            float ic = EvalTopGlyph(px, py);
            if (ic > 0.0f) {
                float icR = isPressed ? 255.0f : 238.0f;
                float icG = isPressed ? 242.0f : 235.0f;
                float icB = isPressed ? 205.0f : 228.0f;
                r = r * (1.0f - ic) + icR * ic;
                g = g * (1.0f - ic) + icG * ic;
                b = b * (1.0f - ic) + icB * ic;
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

static void WriteExpSphereTGA(const char* path, bool isFilled, float cr, float cg, float cb) {
    if (access(path, F_OK) == 0)
        return;
    const int size = 28;
    FILE* fp = fopen(path, "wb");
    if (!fp) return;

    uint8_t hdr[18] = {0};
    hdr[2] = 2; hdr[12] = size; hdr[14] = size; hdr[16] = 32; hdr[17] = 0x28;
    fwrite(hdr, 1, 18, fp);

    std::vector<uint8_t> pix((size_t)size * (size_t)size * 4, 0);
    float half = size * 0.5f;
    float outerR = 12.4f;
    float innerR = 10.4f;

    for (int y = 0; y < size; ++y) {
        float py = (y + 0.5f) - half;
        float ny = py / innerR;
        for (int x = 0; x < size; ++x) {
            float px = (x + 0.5f) - half;
            float nx = px / innerR;
            float d = hypotf(px, py);
            size_t i = ((size_t)y * size + (size_t)x) * 4;

            float r = 10.0f, g = 10.0f, b = 12.0f, a = 0.0f;
            if (d <= outerR + 1.2f) {
                float mask = 1.0f - SmoothStepF(outerR - 0.6f, outerR + 1.2f, d);
                float nr = fminf(1.0f, d / innerR);
                float sphereZ = sqrtf(fmaxf(0.0f, 1.0f - nr * nr));

                float crescentEnv = hypotf((nx + 0.18f) * 1.15f, (ny + 0.45f) * 1.75f);
                float topCrescent = (1.0f - SmoothStepF(0.15f, 0.72f, crescentEnv)) * SmoothStepF(0.0f, -0.75f, ny);
                float hotSpec = powf(fmaxf(0.0f, -0.45f * nx - 0.55f * ny + sphereZ * 0.62f), 3.6f);

                if (!isFilled) {
                    float botBounce = SmoothStepF(0.15f, 0.90f, ny) * nr * 20.0f;
                    float glass = fminf(1.0f, topCrescent * 0.52f + hotSpec * 0.42f);
                    r = 16.0f + botBounce + glass * 150.0f;
                    g = 16.0f + botBounce + glass * 148.0f;
                    b = 18.0f + botBounce + glass * 142.0f;
                    a = mask * 238.0f;

                    float rimD = fabsf(d - (outerR - 1.1f));
                    if (rimD < 1.4f) {
                        float rm = 1.0f - rimD / 1.4f;
                        float grad = 0.65f + 0.35f * (1.0f - (float)(x + y) / (2.0f * size));
                        r = r * (1.0f - rm) + (162.0f * grad) * rm;
                        g = g * (1.0f - rm) + (144.0f * grad) * rm;
                        b = b * (1.0f - rm) + (114.0f * grad) * rm;
                        a = fmaxf(a, rm * 250.0f);
                    }
                } else if (d <= innerR + 0.7f) {
                    float fillMask = 1.0f - SmoothStepF(innerR - 0.6f, innerR + 0.7f, d);
                    float core = 0.42f + 0.58f * sphereZ;
                    float caustic = SmoothStepF(0.20f, 0.85f, ny) * (1.0f - nr * 0.5f) * 46.0f;
                    float spec = hotSpec * 115.0f + topCrescent * 75.0f;
                    r = fminf(255.0f, cr * core + caustic + spec);
                    g = fminf(255.0f, cg * core + caustic * 0.85f + spec * 0.95f);
                    b = fminf(255.0f, cb * core + caustic * 0.40f + spec * 0.82f);
                    a = fillMask * 252.0f;
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
    float innerR = 23.2f;

    for (int y = 0; y < size; ++y) {
        float py = (y + 0.5f) - half;
        float ny = py / outerR;
        for (int x = 0; x < size; ++x) {
            float px = (x + 0.5f) - half;
            float nx = px / outerR;
            float d = hypotf(px, py);
            size_t i = ((size_t)y * size + (size_t)x) * 4;

            float r = 14.0f, g = 14.0f, b = 16.0f, a = 0.0f;

            if (d < innerR) {
                float crescentEnv = hypotf(nx * 1.15f, (ny + 0.42f) * 1.85f);
                float topGlass = (1.0f - SmoothStepF(0.15f, 0.68f, crescentEnv)) * SmoothStepF(-0.08f, -0.72f, ny);
                float innerShadow = SmoothStepF(innerR - 2.4f, innerR, d);
                r = 245.0f * (1.0f - innerShadow) + 10.0f * innerShadow;
                g = 245.0f * (1.0f - innerShadow) + 10.0f * innerShadow;
                b = 240.0f * (1.0f - innerShadow) + 12.0f * innerShadow;
                a = fminf(215.0f, topGlass * 48.0f + innerShadow * 185.0f);
            } else if (d < outerR + 2.2f) {
                float sh = 1.0f - SmoothStepF(outerR - 0.5f, outerR + 2.2f, d);
                a = sh * 135.0f;
            }

            if (d >= innerR - 0.8f && d <= outerR) {
                float inMask  = SmoothStepF(innerR - 0.8f, innerR + 0.6f, d);
                float outMask = 1.0f - SmoothStepF(outerR - 0.9f, outerR + 0.4f, d);
                float ringMask = inMask * outMask;

                float grad = 0.65f + 0.35f * (-0.55f * nx - 0.82f * ny);
                if (d < innerR + 1.3f || d > outerR - 1.4f) {
                    r = 162.0f * grad; g = 144.0f * grad; b = 114.0f * grad;
                } else {
                    r = 54.0f * grad;  g = 48.0f * grad;  b = 40.0f * grad;
                }
                a = fmaxf(a, ringMask * 252.0f);
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

// Reverted Status Plate to the full-width rectangular plate from image1.jpeg (196x44)
static void WriteStatusPlateTGA(const char* path) {
    if (access(path, F_OK) == 0) return;
    const int w = 196, h = 44;
    FILE* fp = fopen(path, "wb");
    if (!fp) return;
    uint8_t hdr[18] = {0};
    hdr[2] = 2; hdr[12] = w & 0xFF; hdr[13] = (w >> 8) & 0xFF; hdr[14] = h & 0xFF; hdr[15] = (h >> 8) & 0xFF; hdr[16] = 32; hdr[17] = 0x28;
    fwrite(hdr, 1, 18, fp);

    std::vector<uint8_t> pix((size_t)w * (size_t)h * 4, 0);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            size_t i = ((size_t)y * w + (size_t)x) * 4;

            bool isOuterDark = (y == 0 || y == h - 1 || x == 0 || x == w - 1);
            bool isBevel     = (y == 1 || y == h - 2 || x == 1 || x == w - 2);
            bool isDivider   = (y == 17 || y == 18 || y == 30 || y == 31) && (x >= 18 && x <= w - 3);

            float r = 14.0f, g = 15.0f, b = 18.0f, a = 244.0f;

            if (isOuterDark) {
                r = 12.0f; g = 13.0f; b = 15.0f; a = 248.0f;
            } else if (isBevel) {
                float grad = (y == 1 || x == w - 2) ? 1.12f : (0.72f + 0.28f * (1.0f - (float)y / (float)h));
                r = fminf(255.0f, 156.0f * grad);
                g = fminf(255.0f, 140.0f * grad);
                b = fminf(255.0f, 112.0f * grad);
                a = 254.0f;
            } else if (isDivider) {
                float divGrad = (y == 17 || y == 30) ? 0.85f : 0.40f;
                r = 132.0f * divGrad;
                g = 118.0f * divGrad;
                b =  96.0f * divGrad;
                a = 248.0f;
            } else {
                bool inSlot = (y >= 4 && y <= 16) || (y >= 19 && y <= 29) || (y >= 32 && y <= 39);
                if (inSlot && x >= 22 && x <= w - 4) {
                    r = 8.0f; g = 9.0f; b = 11.0f; a = 248.0f;
                } else {
                    r = 22.0f; g = 22.0f; b = 24.0f; a = 246.0f;
                }
            }
            pix[i + 0] = ClampU8(b); pix[i + 1] = ClampU8(g); pix[i + 2] = ClampU8(r); pix[i + 3] = ClampU8(a);
        }
    }
    fwrite(pix.data(), 1, pix.size(), fp);
    fclose(fp);
}

// Full-width rectangular HP/SP/ST Bar Fill with 8-frame animated silk-liquid caustic wave
static void WriteGlossyBarFillTGA(const char* path, int w, int h, float cr, float cg, float cb, int frameIdx = 0) {
    if (access(path, F_OK) == 0) return;
    FILE* fp = fopen(path, "wb");
    if (!fp) return;
    uint8_t hdr[18] = {0};
    hdr[2] = 2; hdr[12] = w & 0xFF; hdr[13] = (w >> 8) & 0xFF; hdr[14] = h & 0xFF; hdr[15] = (h >> 8) & 0xFF; hdr[16] = 32; hdr[17] = 0x28;
    fwrite(hdr, 1, 18, fp);

    const float phase = ((float)frameIdx / 8.0f) * 6.2831853f;
    std::vector<uint8_t> pix((size_t)w * (size_t)h * 4, 0);

    for (int y = 0; y < h; ++y) {
        float ny = (float)y / (float)(h > 1 ? h - 1 : 1);
        float cyl = 0.52f + 0.48f * sinf(ny * 3.14159265f);
        float topSpec = (ny >= 0.10f && ny <= 0.32f) ? 46.0f : 0.0f;

        for (int x = 0; x < w; ++x) {
            size_t i = ((size_t)y * w + (size_t)x) * 4;

            // Both harmonics travel in the SAME forward direction (-phase) with exact integer cycles across w=171 so 7->0 loops seamlessly
            float wave1 = 0.5f + 0.5f * sinf(((float)x / 57.0f + ny * 0.24f) * 6.2831853f - phase);
            float wave2 = 0.5f + 0.5f * cosf(((float)x / 85.5f - ny * 0.18f) * 6.2831853f - phase);
            float caustic = powf(0.62f * wave1 + 0.38f * wave2, 2.4f) * 40.0f * sinf(ny * 3.14159265f);
            float bodyBoost = 0.88f + 0.20f * wave1;

            float r = fminf(255.0f, cr * cyl * bodyBoost + topSpec + caustic);
            float g = fminf(255.0f, cg * cyl * bodyBoost + topSpec + caustic * 0.95f);
            float b = fminf(255.0f, cb * cyl * bodyBoost + topSpec * 0.90f + caustic * 0.88f);
            pix[i + 0] = ClampU8(b); pix[i + 1] = ClampU8(g); pix[i + 2] = ClampU8(r); pix[i + 3] = 252;
        }
    }
    fwrite(pix.data(), 1, pix.size(), fp);
    fclose(fp);
}

// 3D Glass Sphere Dome + Classic Dark Smoky Collar & Warm Muted Bronze-Taupe Bezel ON TOP of QuickSlots
static void WriteCircularBezelOverlayTGA(const char* path, int size, float holeR, float outerR, float rimW, bool isJadeAffect = false) {
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

    std::vector<uint8_t> pix((size_t)size * (size_t)size * 4, 0);
    float half = size * 0.5f;
    size_t idx = 0;

    for (int y = 0; y < size; ++y) {
        float py = (y + 0.5f) - half;
        float ny = py / outerR;
        for (int x = 0; x < size; ++x) {
            float px = (x + 0.5f) - half;
            float nx = px / outerR;
            float d = hypotf(px, py);

            float r = 12.0f, g = 13.0f, b = 15.0f, a = 0.0f;

            if (d < holeR) {
                float hnx = px / holeR;
                float hny = py / holeR;
                float nr = d / holeR;
                float sphereZ = sqrtf(fmaxf(0.0f, 1.0f - nr * nr));

                // 1. Top Crescent Glass Dome Shade over the skill/potion/weapon icon
                float crescentEnv = hypotf(hnx * 1.12f, (hny + 0.48f) * 1.85f);
                float topCrescent = (1.0f - SmoothStepF(0.16f, 0.76f, crescentEnv)) * SmoothStepF(-0.08f, -0.82f, hny);
                float hotSpec = powf(fmaxf(0.0f, -0.45f * hnx - 0.65f * hny + sphereZ * 0.35f), 4.8f);
                float glassAlphaVal = fminf(128.0f, topCrescent * 92.0f + hotSpec * 52.0f);

                // 2. 3D Spherical Rim Vignette around the icon perimeter
                float innerVig = SmoothStepF(holeR - 5.0f, holeR, d);
                float vigR = 12.0f, vigG = 12.0f, vigB = 14.0f;
                float vigA = innerVig * 220.0f;

                float totalA = fminf(246.0f, glassAlphaVal + vigA);
                if (totalA > 0.5f) {
                    float wGlass = glassAlphaVal / totalA;
                    r = 248.0f * wGlass + vigR * (1.0f - wGlass);
                    g = 244.0f * wGlass + vigG * (1.0f - wGlass);
                    b = 235.0f * wGlass + vigB * (1.0f - wGlass);
                    a = totalA;
                }
            } else if (d <= outerR) {
                float outFade = 1.0f - SmoothStepF(outerR - 1.0f, outerR + 0.4f, d);
                float lGrad = 0.58f + 0.42f * (-0.58f * nx - 0.82f * ny);

                r = 20.0f * lGrad;
                g = 20.0f * lGrad;
                b = 22.0f * lGrad;
                a = 255.0f * outFade;

                float rimR = isJadeAffect ? 120.0f : 162.0f;
                float rimG = isJadeAffect ? 158.0f : 144.0f;
                float rimB = isJadeAffect ? 156.0f : 114.0f;

                float inLipD = fabsf(d - (holeR + 1.1f));
                if (inLipD < 1.5f) {
                    float im = (1.0f - inLipD / 1.5f) * 0.78f;
                    r = r * (1.0f - im) + (rimR * lGrad) * im;
                    g = g * (1.0f - im) + (rimG * lGrad) * im;
                    b = b * (1.0f - im) + (rimB * lGrad) * im;
                }

                float outRimD = fabsf(d - (outerR - rimW * 0.42f));
                if (outRimD < rimW * 0.68f) {
                    float rm = 1.0f - SmoothStepF(rimW * 0.18f, rimW * 0.68f, outRimD);
                    float spec = powf(fmaxf(0.0f, -0.60f * nx - 0.80f * ny), 3.5f) * 34.0f;
                    r = r * (1.0f - rm) + fminf(255.0f, rimR * lGrad + spec) * rm;
                    g = g * (1.0f - rm) + fminf(255.0f, rimG * lGrad + spec * 0.92f) * rm;
                    b = b * (1.0f - rm) + fminf(255.0f, rimB * lGrad + spec * 0.78f) * rm;
                }
            } else if (d < outerR + 2.8f) {
                float sh = 1.0f - SmoothStepF(outerR, outerR + 2.8f, d);
                a = sh * 120.0f;
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

static void WriteWhiteTouchOrbTGA(const char* path) {
    if (access(path, F_OK) == 0)
        return;
    const int size = 96;
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
            float d = hypotf(px, py);

            float a = 0.0f;
            if (d < 26.0f) {
                float core = 1.0f - SmoothStepF(4.0f, 26.0f, d);
                a = fmaxf(a, core * core * 195.0f);
            }
            float ringD = fabsf(d - 36.0f);
            if (ringD < 5.5f) {
                float rm = 1.0f - SmoothStepF(1.0f, 5.5f, ringD);
                a = fmaxf(a, rm * 135.0f);
            }

            pix[idx + 0] = 255;
            pix[idx + 1] = 255;
            pix[idx + 2] = 255;
            pix[idx + 3] = ClampU8(a);
            idx += 4;
        }
    }
    fwrite(pix.data(), 1, pix.size(), fp);
    fclose(fp);
}

enum ETooltipActionIcon {
    TTICON_USE = 1,
    TTICON_SPLIT = 2,
    TTICON_LINK = 3,
    TTICON_CHEST = 4,
    TTICON_PREVIEW = 5,
    TTICON_CLOSE = 6
};

static void WriteTooltipActionIconTGA(const char* path, int iconType, bool isPressed) {
    if (access(path, F_OK) == 0) return;
    const int size = 36;
    FILE* fp = fopen(path, "wb");
    if (!fp) return;
    uint8_t hdr[18] = {0};
    hdr[2] = 2; hdr[12] = size; hdr[14] = size; hdr[16] = 32; hdr[17] = 0x28;
    fwrite(hdr, 1, 18, fp);

    std::vector<uint8_t> pix((size_t)size * (size_t)size * 4, 0);
    float half = size * 0.5f;
    for (int y = 0; y < size; ++y) {
        float py = (y + 0.5f) - half;
        for (int x = 0; x < size; ++x) {
            float px = (x + 0.5f) - half;
            float ax = fabsf(px), ay = fabsf(py);
            int edgeD = std::min(std::min(x, size - 1 - x), std::min(y, size - 1 - y));
            size_t i = ((size_t)y * size + (size_t)x) * 4;

            float r = isPressed ? 92.0f : 24.0f;
            float g = isPressed ? 42.0f : 23.0f;
            float b = isPressed ? 18.0f : 22.0f;
            float a = 242.0f;
            if (edgeD == 0) {
                r = 34.0f; g = 32.0f; b = 30.0f; a = 240.0f;
            } else if (edgeD == 1) {
                r = isPressed ? 185.0f : 146.0f;
                g = isPressed ? 178.0f : 140.0f;
                b = isPressed ? 166.0f : 130.0f;
                a = 255.0f;
            }

            float ic = 0.0f;
            if (iconType == TTICON_USE) {
                float dBlade = DistToSegF(px, py, -7.0f, 7.0f, 7.5f, -7.5f);
                float dGuard = DistToSegF(px, py, -6.5f, 0.5f, -0.5f, 6.5f);
                if (fminf(dBlade, dGuard) < 1.8f) ic = 1.0f - SmoothStepF(0.8f, 1.8f, fminf(dBlade, dGuard));
            } else if (iconType == TTICON_SPLIT) {
                bool boxL = (px >= -10.5f && px <= -3.5f && ay <= 4.5f);
                bool boxR = (px >= 3.5f && px <= 10.5f && ay <= 4.5f);
                float sepLine = (ax < 1.1f && ay < 8.5f) ? 0.95f : 0.0f;
                if (boxL || boxR) {
                    float bEdge = fminf(fabsf(ax - 7.0f) - 2.2f, ay - 3.2f);
                    if (bEdge > -1.4f) ic = 0.96f;
                }
                ic = fmaxf(ic, sepLine);
            } else if (iconType == TTICON_LINK) {
                float bd = hypotf(px / 9.5f, (py + 1.5f) / 7.2f);
                if (bd <= 1.0f && bd >= 0.72f) ic = 0.96f;
                if (fabsf(py + 1.5f) < 1.4f && (fabsf(px) < 1.4f || fabsf(ax - 4.2f) < 1.4f)) ic = 0.96f;
                if (DistToSegF(px, py, -3.0f, 5.0f, -6.5f, 8.5f) < 1.6f) ic = 0.96f;
            } else if (iconType == TTICON_CHEST) {
                if (ax < 8.5f && py >= -6.0f && py <= 6.5f) {
                    if (fabsf(py + 0.8f) > 0.9f && !(ax < 1.8f && fabsf(py + 0.8f) < 2.2f))
                        ic = 0.95f;
                }
            } else if (iconType == TTICON_PREVIEW) {
                float eyeY = 4.8f * (1.0f - (px * px) / 81.0f);
                if (ax <= 9.0f && fabsf(ay - eyeY) < 1.5f) ic = 0.96f;
                if (hypotf(px, py) < 2.8f) ic = 0.98f;
            } else if (iconType == TTICON_CLOSE) {
                float d1 = DistToSegF(px, py, -6.5f, -6.5f, 6.5f, 6.5f);
                float d2 = DistToSegF(px, py, 6.5f, -6.5f, -6.5f, 6.5f);
                if (fminf(d1, d2) < 1.8f) ic = 1.0f - SmoothStepF(0.8f, 1.8f, fminf(d1, d2));
            }

            if (ic > 0.0f) {
                float ir = (iconType == TTICON_CLOSE) ? 255.0f : (isPressed ? 255.0f : 245.0f);
                float ig = (iconType == TTICON_CLOSE) ? 115.0f : (isPressed ? 225.0f : 235.0f);
                float ib = (iconType == TTICON_CLOSE) ? 115.0f : (isPressed ? 140.0f : 200.0f);
                r = r * (1.0f - ic) + ir * ic;
                g = g * (1.0f - ic) + ig * ic;
                b = b * (1.0f - ic) + ib * ic;
            }
            pix[i + 0] = ClampU8(b); pix[i + 1] = ClampU8(g); pix[i + 2] = ClampU8(r); pix[i + 3] = ClampU8(a);
        }
    }
    fwrite(pix.data(), 1, pix.size(), fp);
    fclose(fp);
}

static void WriteUtilitySystemIconTGA(const char* path, int sysType, float ar, float ag, float ab) {
    if (access(path, F_OK) == 0)
        return;
    const int size = 88;
    FILE* fp = fopen(path, "wb");
    if (!fp) return;

    uint8_t hdr[18] = {0};
    hdr[2] = 2; hdr[12] = size; hdr[14] = size; hdr[16] = 32; hdr[17] = 0x28;
    fwrite(hdr, 1, 18, fp);

    std::vector<uint8_t> pix((size_t)size * (size_t)size * 4, 0);
    float half = size * 0.5f;
    float outerR = 41.0f;
    float rimW = 3.0f;
    float s = outerR / 28.0f;

    auto EvalSysGlyph = [&](float ix, float iy, float& outR, float& outG, float& outB) -> float {
        outR = 235.0f; outG = 205.0f; outB = 145.0f;
        float ux = ix / s, uy = iy / s;
        float ax = fabsf(ux), ay = fabsf(uy);
        float ud = hypotf(ux, uy);
        float m = 0.0f;

        if (sysType == 0) {
            float ringD = fabsf(ud - 14.2f);
            if (ringD < 1.6f && ax > 2.8f && ay > 2.8f) {
                m = 1.0f - SmoothStepF(0.6f, 1.6f, ringD);
                outR = 255.0f; outG = 85.0f; outB = 75.0f;
            }
            float d1 = DistToSegF(ux, uy, -12.0f, 12.0f, 12.0f, -12.0f);
            float d2 = DistToSegF(ux, uy, 12.0f, 12.0f, -12.0f, -12.0f);
            float g1 = DistToSegF(ux, uy, -9.5f, 2.5f, -2.5f, 9.5f);
            float g2 = DistToSegF(ux, uy,  2.5f, 9.5f,  9.5f, 2.5f);
            float md = fminf(fminf(d1, d2), fminf(g1, g2));
            if (md < 2.3f) {
                float sm = 1.0f - SmoothStepF(0.9f, 2.3f, md);
                if (sm >= m) {
                    m = sm;
                    bool isGuard = (fminf(g1, g2) < fminf(d1, d2));
                    outR = isGuard ? 245.0f : 235.0f;
                    outG = isGuard ? 195.0f : 230.0f;
                    outB = isGuard ?  95.0f : 225.0f;
                }
            }
        } else if (sysType == 1) {
            float orbD = fabsf(ud - 13.5f);
            if (orbD < 2.1f && ax > 2.4f) {
                m = 1.0f - SmoothStepF(0.8f, 2.1f, orbD);
                outR = 195.0f; outG = 135.0f; outB = 255.0f;
            }
            float a1 = DistToSegF(ux, uy,  2.0f, -13.5f,  6.8f, -9.4f);
            float a2 = DistToSegF(ux, uy, -2.0f,  13.5f, -6.8f,  9.4f);
            if (fminf(a1, a2) < 2.1f) {
                m = fmaxf(m, 1.0f - SmoothStepF(0.8f, 2.1f, fminf(a1, a2)));
                outR = 215.0f; outG = 155.0f; outB = 255.0f;
            }
            float starD = fminf(ax * 0.52f + ay, ax + ay * 0.52f);
            if (starD <= 6.8f) {
                float sm = 1.0f - SmoothStepF(5.0f, 6.8f, starD);
                if (sm >= m) {
                    m = sm;
                    outR = 255.0f; outG = 225.0f; outB = 115.0f;
                }
            }
        } else if (sysType == 2) {
            float wheelD = fabsf(ud - 13.8f);
            if (wheelD < 1.4f) {
                m = 0.88f;
                outR = 215.0f; outG = 175.0f; outB = 95.0f;
            }
            const float eCols[6][3] = {
                {255, 75, 75}, {75, 185, 255}, {75, 235, 135},
                {255, 215, 75}, {195, 115, 255}, {245, 240, 230}
            };
            for (int k = 0; k < 6; ++k) {
                float ang = k * 1.0471975f;
                float nd = hypotf(ux - cosf(ang) * 13.8f, uy - sinf(ang) * 13.8f);
                if (nd < 3.0f) {
                    float nm = 1.0f - SmoothStepF(1.8f, 3.0f, nd);
                    if (nm >= m) {
                        m = nm;
                        outR = eCols[k][0]; outG = eCols[k][1]; outB = eCols[k][2];
                    }
                }
            }
            float gemD = ax * 1.10f + ay * 0.85f;
            if (gemD <= 8.6f) {
                float gm = 1.0f - SmoothStepF(7.2f, 8.6f, gemD);
                if (gm >= m) {
                    m = gm;
                    float facet = (gemD > 4.8f && gemD < 5.8f) ? 0.55f : 1.0f;
                    outR = 255.0f * facet; outG = 205.0f * facet; outB = 95.0f * facet;
                }
            }
        } else if (sysType == 3) {
            float lx = ux + 2.5f, ly = uy + 2.5f;
            float ld = hypotf(lx, ly);
            float rimD = fabsf(ld - 9.6f);
            if (rimD < 2.4f) {
                m = 1.0f - SmoothStepF(1.1f, 2.4f, rimD);
                outR = 242.0f; outG = 198.0f; outB = 108.0f;
            } else if (ld < 7.4f) {
                float star = fminf(fabsf(lx) * 0.55f + fabsf(ly), fabsf(lx) + fabsf(ly) * 0.55f);
                if (star < 4.8f) {
                    m = 1.0f - SmoothStepF(3.2f, 4.8f, star);
                    outR = 95.0f; outG = 245.0f; outB = 220.0f;
                }
            }
            float hdl = DistToSegF(ux, uy, 4.4f, 4.4f, 13.2f, 13.2f);
            if (hdl < 2.6f) {
                float hm = 1.0f - SmoothStepF(1.3f, 2.6f, hdl);
                if (hm > m) {
                    m = hm;
                    outR = 195.0f; outG = 142.0f; outB = 78.0f;
                }
            }
        } else if (sysType == 4) {
            float c1 = hypotf(ux + 4.4f, uy + 3.4f);
            float c2 = hypotf(ux - 4.4f, uy - 3.4f);
            if (c2 <= 8.2f) {
                m = 1.0f - SmoothStepF(7.0f, 8.2f, c2);
                float ring = (fabsf(c2 - 5.4f) < 0.85f || c2 < 2.0f) ? 0.55f : 1.0f;
                outR = 255.0f * ring; outG = 208.0f * ring; outB = 82.0f * ring;
            } else if (c1 <= 8.0f && c2 > 8.8f) {
                m = 1.0f - SmoothStepF(6.8f, 8.0f, c1);
                float ring = (fabsf(c1 - 5.2f) < 0.85f) ? 0.55f : 1.0f;
                outR = 125.0f * ring; outG = 215.0f * ring; outB = 255.0f * ring;
            }
            float arr1 = DistToSegF(ux, uy,  3.5f, -11.0f, 11.0f, -3.5f);
            float arr2 = DistToSegF(ux, uy, -11.0f,  3.5f, -3.5f, 11.0f);
            if (fminf(arr1, arr2) < 1.8f) {
                m = fmaxf(m, 0.96f);
                outR = 255.0f; outG = 245.0f; outB = 215.0f;
            }
        } else if (sysType == 5) {
            float b1x = ux + 3.6f, b1y = uy + 3.0f;
            float b2x = ux - 3.4f, b2y = uy - 3.0f;
            bool inFront = (fabsf(b2x) <= 8.2f && fabsf(b2y) <= 6.8f);
            bool inBack  = (fabsf(b1x) <= 7.8f && fabsf(b1y) <= 6.5f) && !(fabsf(b2x) <= 9.2f && fabsf(b2y) <= 7.8f);
            if (inFront) {
                m = 0.98f;
                if (fabsf(b2y + 1.0f) < 0.85f && fabsf(b2x) > 2.1f) {
                    m = 0.15f;
                } else if (fabsf(b2x) <= 2.1f && fabsf(b2y + 1.0f) <= 2.3f) {
                    outR = 255.0f; outG = 225.0f; outB = 115.0f;
                } else {
                    outR = 225.0f; outG = 172.0f; outB = 98.0f;
                }
            } else if (inBack) {
                m = 0.94f;
                outR = 165.0f; outG = 128.0f; outB = 82.0f;
                if (fabsf(b1y + 1.0f) < 0.85f) m = 0.15f;
            }
        } else if (sysType == 6) {
            if (ax <= 11.6f && ay <= 10.6f) {
                m = 0.98f;
                float border = fmaxf(ax - 9.2f, ay - 8.2f);
                if (border > 0.0f) {
                    outR = 195.0f; outG = 205.0f; outB = 218.0f;
                } else if (fabsf(border + 0.8f) < 0.65f) {
                    outR = 55.0f; outG = 60.0f; outB = 68.0f;
                } else {
                    if (ud < 2.2f || (ax < 1.15f && uy >= 1.5f && uy <= 6.0f)) {
                        m = 0.12f;
                    } else if (fabsf(ud - 5.0f) < 1.35f) {
                        outR = 255.0f; outG = 212.0f; outB = 98.0f;
                    } else {
                        outR = 148.0f; outG = 162.0f; outB = 178.0f;
                    }
                }
            }
        } else if (sysType == 7) {
            float padD = hypotf(ux * 0.84f, (uy - 4.4f) * 1.08f) - 5.8f;
            float t1 = hypotf((ux + 8.0f) * 1.12f, (uy + 2.2f) * 0.88f) - 3.0f;
            float t2 = hypotf((ux + 2.9f) * 1.12f, (uy + 7.0f) * 0.88f) - 3.2f;
            float t3 = hypotf((ux - 2.9f) * 1.12f, (uy + 7.0f) * 0.88f) - 3.2f;
            float t4 = hypotf((ux - 8.0f) * 1.12f, (uy + 2.2f) * 0.88f) - 3.0f;
            float minD = fminf(padD, fminf(fminf(t1, t2), fminf(t3, t4)));
            if (minD < 0.8f) {
                m = 1.0f - SmoothStepF(-0.5f, 0.8f, minD);
                float bev = 1.15f - 0.15f * SmoothStepF(-3.0f, 0.5f, minD);
                outR = fminf(255.0f, 238.0f * bev);
                outG = fminf(255.0f, 178.0f * bev);
                outB = fminf(255.0f,  88.0f * bev);
            }
        } else if (sysType == 8) {
            float archR = hypotf(ax, fminf(0.0f, uy + 1.5f));
            float armD = (uy > -1.5f) ? fabsf(ax - 8.0f) : fabsf(archR - 8.0f);
            if (uy <= 9.8f && armD <= 3.0f) {
                m = 1.0f - SmoothStepF(2.0f, 3.0f, armD);
                if (uy > 4.5f) {
                    outR = 230.0f; outG = 235.0f; outB = 242.0f;
                } else if (fabsf(uy - 4.2f) < 0.7f) {
                    outR = 45.0f; outG = 25.0f; outB = 25.0f;
                } else {
                    outR = 235.0f; outG = 68.0f; outB = 62.0f;
                }
            }
            float spk = fminf(ax * 0.5f + fabsf(uy - 10.8f), ax + fabsf(uy - 10.8f) * 0.5f);
            if (spk < 3.6f) {
                float sm = 1.0f - SmoothStepF(2.0f, 3.6f, spk);
                if (sm >= m) {
                    m = sm;
                    outR = 255.0f; outG = 225.0f; outB = 95.0f;
                }
            }
        } else if (sysType == 10) {
            float eyeY = 6.8f * (1.0f - (ux * ux) / 152.0f);
            if (ax <= 12.4f && fabsf(ay - eyeY) <= 2.0f) {
                m = 1.0f - SmoothStepF(1.0f, 2.0f, fabsf(ay - eyeY));
                outR = 240.0f; outG = 198.0f; outB = 112.0f;
            }
            if (ud <= 5.0f) {
                float em = 1.0f - SmoothStepF(3.8f, 5.0f, ud);
                if (em >= m) {
                    m = em;
                    if (ud < 1.9f) {
                        outR = 25.0f; outG = 35.0f; outB = 48.0f;
                    } else {
                        outR = 95.0f; outG = 215.0f; outB = 255.0f;
                    }
                }
            }
        }
        return m;
    };

    size_t idx = 0;
    for (int y = 0; y < size; ++y) {
        float py = (y + 0.5f) - half;
        float ny = py / outerR;
        for (int x = 0; x < size; ++x) {
            float px = (x + 0.5f) - half;
            float nx = px / outerR;
            float d = hypotf(px, py);
            float grain = StoneGrainNoise(x, y);

            float r = 12.0f, g = 8.0f, b = 6.0f, a = 0.0f;
            if (d < outerR + 2.6f) {
                float sh = 1.0f - SmoothStepF(outerR - 0.4f, outerR + 2.6f, d);
                a = sh * 145.0f;
            }

            float innerEdge = outerR - rimW;
            if (d <= outerR) {
                float bodyMask = 1.0f - SmoothStepF(outerR - 0.9f, outerR + 0.4f, d);
                float radRatio = fminf(1.0f, d / innerEdge);
                float sphereZ = sqrtf(fmaxf(0.0f, 1.0f - radRatio * radRatio));

                float botGlow = SmoothStepF(-0.20f, 0.88f, ny) * (0.30f + 0.70f * radRatio);
                r = 18.0f + botGlow * ar * 0.42f + grain * 4.0f;
                g = 13.0f + botGlow * ag * 0.36f + grain * 3.0f;
                b = 10.0f + botGlow * ab * 0.32f + grain * 3.0f;

                float crescentEnv = hypotf(nx * 1.12f, (ny + 0.46f) * 1.78f);
                float topCrescent = (1.0f - SmoothStepF(0.16f, 0.76f, crescentEnv)) * SmoothStepF(-0.05f, -0.78f, ny);
                float hotSpot = powf(fmaxf(0.0f, -0.45f * nx - 0.65f * ny + sphereZ * 0.35f), 4.5f);
                float glass = fminf(1.0f, topCrescent * 0.50f + hotSpot * 0.44f);
                r = r * (1.0f - glass * 0.48f) + 248.0f * (glass * 0.48f);
                g = g * (1.0f - glass * 0.48f) + 232.0f * (glass * 0.48f);
                b = b * (1.0f - glass * 0.48f) + 208.0f * (glass * 0.48f);
                a = fmaxf(a, bodyMask * 250.0f);

                float rimDist = fabsf(d - (outerR - rimW * 0.5f));
                if (rimDist < rimW * 0.88f) {
                    float rm = 1.0f - SmoothStepF(rimW * 0.25f, rimW * 0.88f, rimDist);
                    float lGrad = 0.58f + 0.42f * (-0.55f * nx - 0.82f * ny);
                    r = r * (1.0f - rm) + fminf(255.0f, 198.0f * lGrad + ar * 0.12f) * rm;
                    g = g * (1.0f - rm) + fminf(255.0f, 158.0f * lGrad + ag * 0.12f) * rm;
                    b = b * (1.0f - rm) + fminf(255.0f, 102.0f * lGrad + ab * 0.12f) * rm;
                    a = fmaxf(a, rm * 254.0f);
                }
            }

            float dr = 0, dg = 0, db = 0;
            float shMask = EvalSysGlyph(px - 1.5f, py - 1.8f, dr, dg, db);
            if (shMask > 0.05f) {
                float sm = shMask * 0.80f;
                r *= (1.0f - sm); g *= (1.0f - sm); b *= (1.0f - sm);
            }

            float icR = 235.0f, icG = 205.0f, icB = 145.0f;
            float ic = EvalSysGlyph(px, py, icR, icG, icB);
            if (ic > 0.0f) {
                float vGrad = 0.85f + 0.30f * (0.5f - 0.5f * ny);
                r = r * (1.0f - ic) + fminf(255.0f, icR * vGrad) * ic;
                g = g * (1.0f - ic) + fminf(255.0f, icG * vGrad) * ic;
                b = b * (1.0f - ic) + fminf(255.0f, icB * vGrad) * ic;
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

static void WriteMobileStatusHUDTextures() {
    WriteMobileRingTGA("mark/m_portrait_bg.tga", 64, 28.5f, 2.2f, 238.0f, 162,144,114, 212,188,144, MICON_NONE, false);
    WritePortraitRingTGA("mark/m_portrait_ring.tga");
    WriteExpSphereTGA("mark/m_exp_orb_empty_v3.tga", false, 0.0f, 0.0f, 0.0f);
    WriteExpSphereTGA("mark/m_exp_orb_full_v3.tga",  true, 232.0f, 184.0f, 42.0f);
    WriteExpSphereTGA("mark/m_exp_orb_conq_v3.tga",  true,  65.0f, 198.0f, 248.0f);
    WriteStatusPlateTGA("mark/m_hud_plate_v4.tga");
    for (int f = 0; f < 8; ++f) {
        char hpPath[64], spPath[64], enPath[64];
        snprintf(hpPath, sizeof(hpPath), "mark/m_hud_hp_v4_%d.tga", f);
        snprintf(spPath, sizeof(spPath), "mark/m_hud_sp_v4_%d.tga", f);
        snprintf(enPath, sizeof(enPath), "mark/m_hud_en_v4_%d.tga", f);
        // Full 171px width for all 3 bars matching image1.jpeg
        WriteGlossyBarFillTGA(hpPath, 171, 13, 218.0f,  32.0f,  32.0f, f);
        WriteGlossyBarFillTGA(spPath, 171, 11,  32.0f, 122.0f, 235.0f, f);
        WriteGlossyBarFillTGA(enPath, 171,  8, 224.0f, 174.0f,  38.0f, f);
    }

    WriteCircularBezelOverlayTGA("mark/m_slot_bezel_v6.tga",   144, 46.0f, 67.5f, 4.0f, false);
    WriteCircularBezelOverlayTGA("mark/m_affect_bezel_v6.tga",  52, 18.2f, 25.4f, 2.6f, true);
    WriteWhiteTouchOrbTGA("mark/m_touch_orb_v7.tga");

    WriteTooltipActionIconTGA("mark/m_tt_use_norm.tga",     TTICON_USE,     false);
    WriteTooltipActionIconTGA("mark/m_tt_use_down.tga",     TTICON_USE,     true);
    WriteTooltipActionIconTGA("mark/m_tt_split_norm.tga",   TTICON_SPLIT,   false);
    WriteTooltipActionIconTGA("mark/m_tt_split_down.tga",   TTICON_SPLIT,   true);
    WriteTooltipActionIconTGA("mark/m_tt_link_norm.tga",    TTICON_LINK,    false);
    WriteTooltipActionIconTGA("mark/m_tt_link_down.tga",    TTICON_LINK,    true);
    WriteTooltipActionIconTGA("mark/m_tt_chest_norm.tga",   TTICON_CHEST,   false);
    WriteTooltipActionIconTGA("mark/m_tt_chest_down.tga",   TTICON_CHEST,   true);
    WriteTooltipActionIconTGA("mark/m_tt_preview_norm.tga", TTICON_PREVIEW, false);
    WriteTooltipActionIconTGA("mark/m_tt_preview_down.tga", TTICON_PREVIEW, true);
    WriteTooltipActionIconTGA("mark/m_tt_close_norm.tga",   TTICON_CLOSE,   false);
    WriteTooltipActionIconTGA("mark/m_tt_close_down.tga",   TTICON_CLOSE,   true);

    WriteUtilitySystemIconTGA("mark/m_util_icon_0.tga",  0,  220.0f,  55.0f,  50.0f);
    WriteUtilitySystemIconTGA("mark/m_util_icon_1.tga",  1,  165.0f,  85.0f, 235.0f);
    WriteUtilitySystemIconTGA("mark/m_util_icon_2.tga",  2,  225.0f, 165.0f,  55.0f);
    WriteUtilitySystemIconTGA("mark/m_util_icon_3.tga",  3,   55.0f, 195.0f, 175.0f);
    WriteUtilitySystemIconTGA("mark/m_util_icon_4.tga",  4,  215.0f, 175.0f,  65.0f);
    WriteUtilitySystemIconTGA("mark/m_util_icon_5.tga",  5,  195.0f, 145.0f,  85.0f);
    WriteUtilitySystemIconTGA("mark/m_util_icon_6.tga",  6,  135.0f, 155.0f, 185.0f);
    WriteUtilitySystemIconTGA("mark/m_util_icon_7.tga",  7,  225.0f, 135.0f,  60.0f);
    WriteUtilitySystemIconTGA("mark/m_util_icon_8.tga",  8,  215.0f,  75.0f,  75.0f);
    WriteUtilitySystemIconTGA("mark/m_util_icon_10.tga", 10,  75.0f, 175.0f, 235.0f);
}

void GenerateMobileHUDTextures() {
    if (access("mark/theme_wo_classic_v16.flag", F_OK) != 0) {
        static const char* s_oldTgas[] = {
            "mark/m_minimap_frame.tga", "mark/m_portrait_bg.tga", "mark/m_portrait_ring.tga",
            "mark/m_hud_plate_v2.tga", "mark/m_hud_plate_v3.tga", "mark/m_hud_plate_v4.tga",
            "mark/m_exp_orb_empty_v3.tga", "mark/m_exp_orb_full_v3.tga", "mark/m_exp_orb_conq_v3.tga",
            "mark/m_joy_bg_v2.tga", "mark/m_joy_knob_norm_v2.tga", "mark/m_joy_knob_down_v2.tga",
            "mark/m_atk_norm_v3.tga", "mark/m_atk_down_v3.tga", "mark/m_atk_norm_v5.tga", "mark/m_atk_down_v5.tga",
            "mark/m_slot_norm_v3.tga", "mark/m_slot_over_v3.tga",
            "mark/m_slot_bezel_v6.tga", "mark/m_affect_bezel_v6.tga", "mark/m_touch_orb_v7.tga",
            "mark/m_btn_lock_norm_v3.tga", "mark/m_btn_lock_down_v3.tga",
            "mark/m_btn_loot_norm_v3.tga", "mark/m_btn_loot_down_v3.tga",
            "mark/m_btn_page_norm_v3.tga", "mark/m_btn_page_down_v3.tga",
            "mark/m_btn_mount_norm_v3.tga", "mark/m_btn_mount_down_v3.tga",
            "mark/m_btn_chat_norm_v3.tga", "mark/m_btn_chat_down_v3.tga",
            "mark/m_btn_emote_norm_v6.tga", "mark/m_btn_emote_down_v6.tga",
            "mark/m_btn_auto_norm_v6.tga", "mark/m_btn_auto_down_v6.tga",
            "mark/m_btn_quest_norm_v6.tga", "mark/m_btn_quest_down_v6.tga",
            "mark/m_mm_plus_norm.tga", "mark/m_mm_plus_down.tga",
            "mark/m_mm_minus_norm.tga", "mark/m_mm_minus_down.tga",
            "mark/m_top_collapse_norm_v4.tga", "mark/m_top_collapse_down_v4.tga",
            "mark/m_top_expand_norm_v4.tga", "mark/m_top_expand_down_v4.tga",
            "mark/m_top_char_norm_v4.tga", "mark/m_top_char_down_v4.tga",
            "mark/m_top_inven_norm_v4.tga", "mark/m_top_inven_down_v4.tga",
            "mark/m_top_comm_norm_v4.tga", "mark/m_top_comm_down_v4.tga",
            "mark/m_top_shop_norm_v4.tga", "mark/m_top_shop_down_v4.tga",
            "mark/m_top_auto_norm_v4.tga", "mark/m_top_auto_down_v4.tga",
            "mark/m_top_ds_norm_v4.tga", "mark/m_top_ds_down_v4.tga",
            "mark/m_top_util_norm_v4.tga", "mark/m_top_util_down_v4.tga",
            "mark/m_top_sys_norm_v4.tga", "mark/m_top_sys_down_v4.tga",
            "mark/m_top_spec_norm_v4.tga", "mark/m_top_spec_down_v4.tga"
        };
        for (const char* p : s_oldTgas) unlink(p);
        for (int u = 0; u <= 10; ++u) {
            char upath[64];
            snprintf(upath, sizeof(upath), "mark/m_util_icon_%d.tga", u);
            unlink(upath);
        }
        for (int f = 0; f < 8; ++f) {
            char p1[64], p2[64], p3[64];
            snprintf(p1, sizeof(p1), "mark/m_hud_hp_v4_%d.tga", f);
            snprintf(p2, sizeof(p2), "mark/m_hud_sp_v4_%d.tga", f);
            snprintf(p3, sizeof(p3), "mark/m_hud_en_v4_%d.tga", f);
            unlink(p1); unlink(p2); unlink(p3);
        }
        if (FILE* f = fopen("mark/theme_wo_classic_v16.flag", "wb")) fclose(f);
    }

    // Reverted Joystick Base & Dark Smoky Knob with Warm Muted Bronze-Taupe Rims (matching image1.jpeg)
    WriteMobileRingTGA("mark/m_joy_bg_v2.tga",        160, 72.0f, 3.6f, 155.0f, 162,144,114, 212,188,144, MICON_JOY_BG,      false);
    WriteMobileRingTGA("mark/m_joy_knob_norm_v2.tga",  76, 31.0f, 3.2f, 225.0f, 162,144,114, 212,188,144, MICON_JOY_KNOB,    false);
    WriteMobileRingTGA("mark/m_joy_knob_down_v2.tga",  76, 33.0f, 3.6f, 248.0f, 212,188,144, 242,218,172, MICON_JOY_KNOB,    true);

    WriteSquaredMiniMapTextures();
    WriteMobileStatusHUDTextures();

    WriteMobileRingTGA("mark/m_mm_plus_norm.tga",  24, 10.5f, 1.4f, 225.0f, 162,144,114, 212,188,144, MICON_PLUS,  false);
    WriteMobileRingTGA("mark/m_mm_plus_down.tga",  24, 11.0f, 1.8f, 248.0f, 212,188,144, 242,218,172, MICON_PLUS,  true);
    WriteMobileRingTGA("mark/m_mm_minus_norm.tga", 24, 10.5f, 1.4f, 225.0f, 162,144,114, 212,188,144, MICON_MINUS, false);
    WriteMobileRingTGA("mark/m_mm_minus_down.tga", 24, 11.0f, 1.8f, 248.0f, 212,188,144, 242,218,172, MICON_MINUS, true);

    WriteTopBarButtonTGA("mark/m_top_char_norm_v4.tga",  TICON_CHAR,       false);
    WriteTopBarButtonTGA("mark/m_top_char_down_v4.tga",  TICON_CHAR,       true);
    WriteTopBarButtonTGA("mark/m_top_inven_norm_v4.tga", TICON_INVEN,      false);
    WriteTopBarButtonTGA("mark/m_top_inven_down_v4.tga", TICON_INVEN,      true);
    WriteTopBarButtonTGA("mark/m_top_comm_norm_v4.tga",  TICON_COMM,       false);
    WriteTopBarButtonTGA("mark/m_top_comm_down_v4.tga",  TICON_COMM,       true);
    WriteTopBarButtonTGA("mark/m_top_shop_norm_v4.tga",  TICON_SHOP,       false);
    WriteTopBarButtonTGA("mark/m_top_shop_down_v4.tga",  TICON_SHOP,       true);
    WriteTopBarButtonTGA("mark/m_top_ds_norm_v4.tga",    TICON_DS,         false);
    WriteTopBarButtonTGA("mark/m_top_ds_down_v4.tga",    TICON_DS,         true);
    WriteTopBarButtonTGA("mark/m_top_util_norm_v4.tga",  TICON_UTIL,       false);
    WriteTopBarButtonTGA("mark/m_top_util_down_v4.tga",  TICON_UTIL,       true);
    WriteTopBarButtonTGA("mark/m_top_sys_norm_v4.tga",   TICON_SYS,        false);
    WriteTopBarButtonTGA("mark/m_top_sys_down_v4.tga",   TICON_SYS,        true);
    WriteTopBarButtonTGA("mark/m_top_spec_norm_v4.tga",  TICON_SPEC_BOXES, false);
    WriteTopBarButtonTGA("mark/m_top_spec_down_v4.tga",  TICON_SPEC_BOXES, true);

    // Attack Button uses MICON_NONE (Empty Shaded Slot!) so Python renders the character's equipped weapon icon inside it!
    WriteMobileRingTGA("mark/m_atk_norm_v3.tga",        280, 128.0f,  9.2f, 236.0f, 162,144,114, 212,188,144, MICON_NONE,         false);
    WriteMobileRingTGA("mark/m_atk_down_v3.tga",        280, 132.0f, 10.6f, 250.0f, 212,188,144, 242,218,172, MICON_NONE,         true);
    WriteMobileRingTGA("mark/m_atk_norm_v5.tga",        288, 132.0f,  9.2f, 236.0f, 162,144,114, 212,188,144, MICON_NONE,         false);
    WriteMobileRingTGA("mark/m_atk_down_v5.tga",        288, 136.0f, 10.6f, 252.0f, 212,188,144, 242,218,172, MICON_NONE,         true);
    WriteMobileRingTGA("mark/m_slot_norm_v3.tga",       144,  67.5f,  6.2f, 236.0f, 162,144,114, 212,188,144, MICON_NONE,         false);
    WriteMobileRingTGA("mark/m_slot_over_v3.tga",       144,  69.0f,  7.2f, 248.0f, 212,188,144, 242,218,172, MICON_NONE,         true);
    WriteMobileRingTGA("mark/m_btn_lock_norm_v3.tga",   128,  55.0f,  6.0f, 236.0f, 162,144,114, 212,188,144, MICON_TARGET,       false);
    WriteMobileRingTGA("mark/m_btn_lock_down_v3.tga",   128,  57.0f,  7.0f, 250.0f, 212,188,144, 242,218,172, MICON_TARGET,       true);
    WriteMobileRingTGA("mark/m_btn_loot_norm_v3.tga",   128,  55.0f,  6.0f, 236.0f, 162,144,114, 212,188,144, MICON_LOOT,         false);
    WriteMobileRingTGA("mark/m_btn_loot_down_v3.tga",   128,  57.0f,  7.0f, 250.0f, 212,188,144, 242,218,172, MICON_LOOT,         true);
    WriteMobileRingTGA("mark/m_btn_page_norm_v3.tga",   128,  55.0f,  6.0f, 236.0f, 162,144,114, 212,188,144, MICON_PAGE,         false);
    WriteMobileRingTGA("mark/m_btn_page_down_v3.tga",   128,  57.0f,  7.0f, 250.0f, 212,188,144, 242,218,172, MICON_PAGE,         true);
    WriteMobileRingTGA("mark/m_btn_mount_norm_v3.tga",  128,  57.0f,  6.2f, 236.0f, 162,144,114, 212,188,144, MICON_MOUNT,        false);
    WriteMobileRingTGA("mark/m_btn_mount_down_v3.tga",  128,  59.0f,  7.2f, 250.0f, 212,188,144, 242,218,172, MICON_MOUNT,        true);
    WriteMobileRingTGA("mark/m_btn_chat_norm_v3.tga",   112,  49.0f,  5.6f, 236.0f, 162,144,114, 212,188,144, MICON_CHAT_BUBBLE,  false);
    WriteMobileRingTGA("mark/m_btn_chat_down_v3.tga",   112,  51.0f,  6.8f, 250.0f, 212,188,144, 242,218,172, MICON_CHAT_BUBBLE,  true);
    WriteMobileRingTGA("mark/m_btn_emote_norm_v6.tga",  112,  49.0f,  5.6f, 236.0f, 162,144,114, 212,188,144, MICON_EMOTE_SMILEY, false);
    WriteMobileRingTGA("mark/m_btn_emote_down_v6.tga",  112,  51.0f,  6.8f, 250.0f, 212,188,144, 242,218,172, MICON_EMOTE_SMILEY, true);
    WriteMobileRingTGA("mark/m_btn_auto_norm_v6.tga",   112,  49.0f,  5.6f, 236.0f, 162,144,114, 212,188,144, MICON_AUTO_COMBAT,  false);
    WriteMobileRingTGA("mark/m_btn_auto_down_v6.tga",   112,  51.0f,  6.8f, 250.0f, 212,188,144, 242,218,172, MICON_AUTO_COMBAT,  true);
    WriteMobileRingTGA("mark/m_btn_quest_norm_v6.tga",  112,  49.0f,  5.6f, 236.0f, 162,144,114, 212,188,144, MICON_QUEST_SCROLL, false);
    WriteMobileRingTGA("mark/m_btn_quest_down_v6.tga",  112,  51.0f,  6.8f, 250.0f, 212,188,144, 242,218,172, MICON_QUEST_SCROLL, true);
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
    bool isDraggingWindow = false;
    bool holdDragCancelled = false;
    bool canTouchScroll = false;
    bool isTouchScrolling = false;
    UI::CWindow* pHoldDragWnd = nullptr;
};

struct SScaledWindowRect {
    UI::CWindow* pWnd = nullptr;
    float scale = 1.0f;
    int x = 0, y = 0, w = 0, h = 0;
    uint32_t lastSeenFrame = 0;
};
static std::unordered_map<uintptr_t, SScaledWindowRect> s_scaledWindows;
static UI::CWindow* s_pCachedUiRootWindow = nullptr;
static uint32_t s_uMobileRenderFrame = 1;
static D3DXMATRIX s_savedUiViewMat;
static bool s_bUiScaleActive = false;
static int s_iLastTouchDownX = -1000;
static int s_iLastTouchDownY = -1000;
static bool s_bLastTouchSnappedToButton = false;
static bool s_bExplicitKeyboardRequest = false;

struct CAndroidWindowHierarchyAccessor : public UI::CWindow {
    static UI::CWindow* EnsureRootWindow(UI::CWindow* pAny = nullptr) {
        if (s_pCachedUiRootWindow)
            return s_pCachedUiRootWindow;
        if (!pAny && UI::CWindowManager::InstancePtr())
            pAny = UI::CWindowManager::Instance().GetPointWindow();
        if (!pAny && !s_scaledWindows.empty())
            pAny = s_scaledWindows.begin()->second.pWnd;
        if (!pAny)
            return nullptr;
        while (pAny->GetParent())
            pAny = pAny->GetParent();
        s_pCachedUiRootWindow = pAny;
        return s_pCachedUiRootWindow;
    }

    // Side-effect-free point pick: NEVER calls RunMouseMove and NEVER fires OnMouseOverOut on pressed CButtons!
    static UI::CWindow* PickWindowAtReadOnly(UI::CWindow* pWnd, int x, int y, int depth = 0) {
        if (!pWnd) return nullptr;
        if (depth >= 2 && !pWnd->IsShow())
            return nullptr;

        CAndroidWindowHierarchyAccessor* pAcc = static_cast<CAndroidWindowHierarchyAccessor*>(pWnd);
        for (auto it = pAcc->m_pChildList.rbegin(); it != pAcc->m_pChildList.rend(); ++it) {
            UI::CWindow* pChild = *it;
            // Note: at depth == 0, pChild is a CLayer whose m_bShow is false; only check IsShow() for depth >= 1!
            if (!pChild || (depth >= 1 && !pChild->IsShow()))
                continue;
            UI::CWindow* pHit = PickWindowAtReadOnly(pChild, x, y, depth + 1);
            if (pHit)
                return pHit;
        }

        if (depth >= 2 && !pWnd->IsFlag(UI::CWindow::FLAG_NOT_PICK)) {
            const RECT& rc = pWnd->GetRect();
            if (x >= rc.left && x <= rc.right && y >= rc.top && y <= rc.bottom)
                return pWnd;
        }
        return nullptr;
    }
};

extern bool s_bUiTouchDown;

static void MapTouchThroughScaledWindows(int inX, int inY, int* outX, int* outY) {
    *outX = inX;
    *outY = inY;

    // Only probe with RunMouseMove when no UI touch is currently held down (ACTION_DOWN),
    // so we NEVER trigger OnMouseOverOut on a pressed CButton during ACTION_MOVE or ACTION_UP!
    UI::CWindow* pDirectTop = nullptr;
    if (!s_bUiTouchDown) {
        UI::CWindowManager::Instance().RunMouseMove(inX, inY);
        pDirectTop = UI::CWindowManager::Instance().GetPointWindow();
        CAndroidWindowHierarchyAccessor::EnsureRootWindow(pDirectTop);
    } else {
        UI::CWindow* pRoot = CAndroidWindowHierarchyAccessor::EnsureRootWindow();
        pDirectTop = pRoot ? CAndroidWindowHierarchyAccessor::PickWindowAtReadOnly(pRoot, inX, inY) : nullptr;
    }

    bool bInTopMostPopup = false;
    for (UI::CWindow* pWalk = pDirectTop; pWalk != nullptr; pWalk = pWalk->GetParent()) {
        if (s_scaledWindows.find((uintptr_t)pWalk) != s_scaledWindows.end()) {
            bInTopMostPopup = false;
            break;
        }
        const char* wName = pWalk->GetName();
        if (wName && (strcmp(wName, "MobileHUDFixed") == 0 || strcmp(wName, "TaskBar") == 0 ||
                      strcmp(wName, "EventCurtain") == 0 || strcmp(wName, "EnergyBar") == 0)) {
            bInTopMostPopup = false;
            break;
        }
        if (wName && strcmp(wName, "TOP_MOST") == 0) {
            bInTopMostPopup = true;
        }
    }
    if (bInTopMostPopup) {
        return;
    }

    uint32_t bestFrame = 0;
    int bestArea = 0x7FFFFFFF;
    int bestOutX = inX, bestOutY = inY;
    bool bFoundScaled = false;

    for (auto it = s_scaledWindows.begin(); it != s_scaledWindows.end(); ) {
        if (s_uMobileRenderFrame > it->second.lastSeenFrame + 60) {
            it = s_scaledWindows.erase(it);
            continue;
        }
        const SScaledWindowRect& r = it->second;
        if (s_uMobileRenderFrame <= r.lastSeenFrame + 2 && fabsf(r.scale - 1.0f) > 0.01f && r.w > 0 && r.h > 0) {
            int scaledW = (int)(r.w * r.scale);
            int scaledH = (int)(r.h * r.scale);
            if (inX >= r.x && inX <= r.x + scaledW && inY >= r.y && inY <= r.y + scaledH) {
                int area = scaledW * scaledH;
                if (!bFoundScaled || r.lastSeenFrame > bestFrame || (r.lastSeenFrame == bestFrame && area < bestArea)) {
                    bFoundScaled = true;
                    bestFrame = r.lastSeenFrame;
                    bestArea = area;
                    bestOutX = r.x + (int)((inX - r.x) / r.scale + 0.5f);
                    bestOutY = r.y + (int)((inY - r.y) / r.scale + 0.5f);
                }
            }
        }
        ++it;
    }

    if (bFoundScaled) {
        *outX = bestOutX;
        *outY = bestOutY;
    }
}

static bool IsClickableButtonWindow(UI::CWindow* pWnd) {
    if (!pWnd) return false;
    if (dynamic_cast<UI::CButton*>(pWnd) != nullptr && dynamic_cast<UI::CDragButton*>(pWnd) == nullptr)
        return true;
    const char* szName = pWnd->GetName();
    if (szName && (strcmp(szName, "ChatModeButton") == 0 || strcmp(szName, "ChatTabButton") == 0))
        return true;
    return false;
}

// Ensures child TextLines/ToolTipTexts inside a CButton never intercept touch, without disabling child buttons inside a CDragButton
static UI::CWindow* SanitizeButtonChildPickFlags(UI::CWindow* pWnd) {
    if (!pWnd || IsClickableButtonWindow(pWnd) || dynamic_cast<UI::CDragButton*>(pWnd) != nullptr)
        return pWnd;

    UI::CWindow* pBtnAncestor = nullptr;
    for (UI::CWindow* pWalk = pWnd->GetParent(); pWalk != nullptr; pWalk = pWalk->GetParent()) {
        if (IsClickableButtonWindow(pWalk)) {
            pBtnAncestor = pWalk;
            break;
        }
        if (dynamic_cast<UI::CDragButton*>(pWalk) != nullptr) {
            break;
        }
    }
    if (pBtnAncestor) {
        for (UI::CWindow* pWalk = pWnd; pWalk != nullptr && pWalk != pBtnAncestor; pWalk = pWalk->GetParent()) {
            if (!IsClickableButtonWindow(pWalk)) {
                pWalk->AddFlag(UI::CWindow::FLAG_NOT_PICK);
            }
        }
        return pBtnAncestor;
    }
    return pWnd;
}

static bool IsScrollOrSliderOrButtonWidget(UI::CWindow* pWnd, bool* outIsDragThumb = nullptr, bool* outIsClickableBtn = nullptr) {
    if (outIsDragThumb) *outIsDragThumb = false;
    if (outIsClickableBtn) *outIsClickableBtn = false;
    pWnd = SanitizeButtonChildPickFlags(pWnd);
    for (UI::CWindow* pCur = pWnd; pCur != nullptr; pCur = pCur->GetParent()) {
        const char* szName = pCur->GetName();
        if (szName && (strcmp(szName, "ChatModeButton") == 0 || strcmp(szName, "ChatTabButton") == 0)) {
            if (outIsClickableBtn) *outIsClickableBtn = true;
            return true;
        }
        if (dynamic_cast<UI::CButton*>(pCur) != nullptr) {
            if (outIsClickableBtn) *outIsClickableBtn = true;
            return true;
        }
        if (szName && strcmp(szName, "ChatWindow:ChatButton") == 0) {
            return false;
        }
        if (dynamic_cast<UI::CDragButton*>(pCur) != nullptr) {
            if (outIsDragThumb) *outIsDragThumb = true;
            return true;
        }
        if (dynamic_cast<UI::CTextLine*>(pCur) != nullptr &&
            !pCur->IsFlag(UI::CWindow::FLAG_NOT_PICK) &&
            UI::CWindowManager::Instance().GetActivateWindow() == pCur) {
            return true;
        }
    }
    return false;
}

static void ApplySmartTouchMagnetism(int inX, int inY, int* ioX, int* ioY, bool* outSnappedBtn, bool* outIsScrollThumb) {
    *ioX = inX;
    *ioY = inY;
    if (outSnappedBtn) *outSnappedBtn = false;
    if (outIsScrollThumb) *outIsScrollThumb = false;

    UI::CWindowManager::Instance().RunMouseMove(inX, inY);
    UI::CWindow* pDirect = SanitizeButtonChildPickFlags(UI::CWindowManager::Instance().GetPointWindow());
    bool isThumb = false, isBtn = false;
    if (pDirect && IsScrollOrSliderOrButtonWidget(pDirect, &isThumb, &isBtn)) {
        if (outSnappedBtn) *outSnappedBtn = isBtn;
        if (outIsScrollThumb) *outIsScrollThumb = isThumb;
        UI::CWindowManager::Instance().RunMouseMove(*ioX, *ioY);
        return;
    }
    // Check if the direct touch landed on an open UI / TOP_MOST window (not a MobileHUDFixed background button)
    bool bDirectIsNonHudWindow = false;
    bool bDirectIsHudWidget = false;
    for (UI::CWindow* pWalk = pDirect; pWalk != nullptr; pWalk = pWalk->GetParent()) {
        const char* wName = pWalk->GetName();
        // Only prevent magnetism from pulling a tap off an item/equipment SlotWindow; allow TitleBar touches to magnetize to its Close (X) button!
        if (wName && (strstr(wName, "Slot") != nullptr || strstr(wName, "slot") != nullptr)) {
            return;
        }
        if (wName && strcmp(wName, "MobileHUDFixed") == 0) {
            bDirectIsHudWidget = true;
        }
        if (wName && (strcmp(wName, "UI") == 0 || strcmp(wName, "TOP_MOST") == 0)) {
            bDirectIsNonHudWindow = true;
        }
    }
    if (bDirectIsHudWidget) bDirectIsNonHudWindow = false;

    UI::CWindow* pBestTarget = nullptr;
    int bestDistSq = 999999;
    int bestCenterX = inX, bestCenterY = inY;
    bool bestIsThumb = false, bestIsBtn = false;

    static const int s_radii[] = { 6, 12, 18, 24 };
    static const float s_dirs[8][2] = {
        {1,0}, {-1,0}, {0,1}, {0,-1},
        {0.707f,0.707f}, {-0.707f,0.707f}, {0.707f,-0.707f}, {-0.707f,-0.707f}
    };

    for (int r : s_radii) {
        for (int d = 0; d < 8; ++d) {
            int px = inX + (int)(s_dirs[d][0] * r);
            int py = inY + (int)(s_dirs[d][1] * r);
            UI::CWindowManager::Instance().RunMouseMove(px, py);
            UI::CWindow* pCand = SanitizeButtonChildPickFlags(UI::CWindowManager::Instance().GetPointWindow());
            if (!pCand) continue;

            // Never snap a touch that landed on an open UI window onto a MobileHUDFixed action button behind it!
            if (bDirectIsNonHudWindow) {
                bool bCandIsHud = false;
                for (UI::CWindow* pW = pCand; pW != nullptr; pW = pW->GetParent()) {
                    const char* wn = pW->GetName();
                    if (wn && strcmp(wn, "MobileHUDFixed") == 0) {
                        bCandIsHud = true;
                        break;
                    }
                }
                if (bCandIsHud) continue;
            }

            UI::CWindow* pControl = nullptr;
            bool candThumb = false, candBtn = false;
            for (UI::CWindow* pWalk = pCand; pWalk != nullptr; pWalk = pWalk->GetParent()) {
                const char* wName = pWalk->GetName();
                if (wName && (strcmp(wName, "ChatModeButton") == 0 || strcmp(wName, "ChatTabButton") == 0)) {
                    pControl = pWalk; candBtn = true; break;
                }
                if (dynamic_cast<UI::CButton*>(pWalk) != nullptr) {
                    pControl = pWalk; candBtn = true; break;
                }
                if (wName && strcmp(wName, "ChatWindow:ChatButton") == 0) {
                    break;
                }
                if (dynamic_cast<UI::CDragButton*>(pWalk) != nullptr) {
                    pControl = pWalk; candThumb = true; break;
                }
            }
            if (!pControl) continue;

            int cw = pControl->GetWidth();
            int ch = pControl->GetHeight();
            if (!candThumb && (cw > 90 || ch > 48)) continue;

            const RECT& rc = pControl->GetRect();
            int cx = (rc.left + rc.right) / 2;
            int cy = (rc.top + rc.bottom) / 2;
            int dx = inX - cx;
            int dy = inY - cy;
            int dSq = dx * dx + dy * dy;
            if (candThumb) dSq -= 120;
            else if (cw <= 24 && ch <= 24) dSq -= 80;

            if (dSq < bestDistSq) {
                bestDistSq = dSq;
                pBestTarget = pControl;
                bestCenterX = cx;
                bestCenterY = cy;
                bestIsThumb = candThumb;
                bestIsBtn = candBtn;
            }
        }
    }

    if (pBestTarget) {
        *ioX = bestCenterX;
        *ioY = bestCenterY;
        if (outSnappedBtn) *outSnappedBtn = bestIsBtn;
        if (outIsScrollThumb) *outIsScrollThumb = bestIsThumb;
    }
    UI::CWindowManager::Instance().RunMouseMove(*ioX, *ioY);
    SanitizeButtonChildPickFlags(UI::CWindowManager::Instance().GetPointWindow());
}

static DWORD Pick3DActorAtScreenPos(int sx, int sy) {
    CPythonCharacterManager* pChrMgr = CPythonCharacterManager::InstancePtr();
    CCamera* pCam = CCameraManager::InstancePtr() ? CCameraManager::Instance().GetCurrentCamera() : nullptr;
    if (!pChrMgr || !pCam) return 0;

    CInstanceBase* pMain = pChrMgr->GetMainInstancePtr();
    const D3DXMATRIX& matV = pCam->GetViewMatrix();
    float fovDeg = 30.0f;
    float tanHalfFov = tanf(fovDeg * 0.5f * (3.14159265f / 180.0f));
    float aspect = (float)g_AndroidScreenWidth / (float)(g_AndroidScreenHeight > 0 ? g_AndroidScreenHeight : 720);

    DWORD bestVID = 0;
    float bestScore = 1e9f;

    for (CPythonCharacterManager::CharacterIterator it = pChrMgr->CharacterInstanceBegin();
         it != pChrMgr->CharacterInstanceEnd(); ++it) {
        CInstanceBase* pkInst = *it;
        if (!pkInst || pkInst == pMain) continue;

        TPixelPosition kPos;
        pkInst->NEW_GetPixelPosition(&kPos);
        float wx = kPos.x;
        float wy = -kPos.y;
        float wzFeet = kPos.z;
        float wzHead = kPos.z + 150.0f;

        auto ProjectPt = [&](float px, float py, float pz, float& outX, float& outY) -> bool {
            float vx = px * matV._11 + py * matV._21 + pz * matV._31 + matV._41;
            float vy = px * matV._12 + py * matV._22 + pz * matV._32 + matV._42;
            float vz = px * matV._13 + py * matV._23 + pz * matV._33 + matV._43;
            float depth = (vz < 0.0f) ? -vz : vz;
            if (depth < 20.0f) return false;
            float ndcX = vx / (depth * tanHalfFov * aspect);
            float ndcY = vy / (depth * tanHalfFov);
            outX = (ndcX * 0.5f + 0.5f) * (float)g_AndroidScreenWidth;
            outY = (0.5f - ndcY * 0.5f) * (float)g_AndroidScreenHeight;
            return true;
        };

        float fx = 0, fy = 0, hx = 0, hy = 0;
        if (!ProjectPt(wx, wy, wzFeet, fx, fy) || !ProjectPt(wx, wy, wzHead, hx, hy))
            continue;

        float minY = fminf(fy, hy) - 28.0f;
        float maxY = fmaxf(fy, hy) + 24.0f;
        float bodyH = fmaxf(48.0f, fabsf(fy - hy));
        float halfW = fmaxf(38.0f, bodyH * 0.48f);
        float cx = 0.5f * (fx + hx);

        if (sx >= cx - halfW && sx <= cx + halfW && sy >= minY && sy <= maxY) {
            float cy = 0.5f * (fy + hy);
            float distSq = (sx - cx) * (sx - cx) + (sy - cy) * (sy - cy);
            if (pkInst->IsPC()) distSq *= 0.7f;
            if (distSq < bestScore) {
                bestScore = distSq;
                bestVID = pkInst->GetVirtualID();
            }
        }
    }
    return bestVID;
}

static bool IsMobileItemHoldOrAttachedActive() {
    bool bActive = false;
    PyObject* pMm = PyImport_AddModule("mouseModule");
    PyObject* pMc = (pMm && PyObject_HasAttrString(pMm, "mouseController")) ? PyObject_GetAttrString(pMm, "mouseController") : nullptr;
    PyObject* pIsAtt = (pMc && pMc != Py_None && PyObject_HasAttrString(pMc, "isAttached")) ? PyObject_CallMethod(pMc, "isAttached", NULL) : nullptr;
    if (pIsAtt && PyObject_IsTrue(pIsAtt)) bActive = true;
    Py_XDECREF(pIsAtt);
    Py_XDECREF(pMc);

    if (!bActive) {
        PyObject* pCi = PyImport_AddModule("constInfo");
        PyObject* pInv = (pCi && PyObject_HasAttrString(pCi, "wndInventory")) ? PyObject_GetAttrString(pCi, "wndInventory") : nullptr;
        PyObject* pPend = (pInv && pInv != Py_None && PyObject_HasAttrString(pInv, "_mobilePendingHoldSlot")) ? PyObject_GetAttrString(pInv, "_mobilePendingHoldSlot") : nullptr;
        if (pPend && pPend != Py_None && PyObject_IsTrue(pPend)) bActive = true;
        Py_XDECREF(pPend);
        Py_XDECREF(pInv);

        if (!bActive && pCi && PyObject_HasAttrString(pCi, "GetInterfaceInstance")) {
            PyObject* pItf = PyObject_CallMethod(pCi, "GetInterfaceInstance", NULL);
            PyObject* pSpec = (pItf && pItf != Py_None && PyObject_HasAttrString(pItf, "wndSpecialStorage")) ? PyObject_GetAttrString(pItf, "wndSpecialStorage") : nullptr;
            PyObject* pSpecPend = (pSpec && pSpec != Py_None && PyObject_HasAttrString(pSpec, "_mobilePendingHoldSlot")) ? PyObject_GetAttrString(pSpec, "_mobilePendingHoldSlot") : nullptr;
            if (pSpecPend && pSpecPend != Py_None && PyObject_IsTrue(pSpecPend)) bActive = true;
            Py_XDECREF(pSpecPend);
            Py_XDECREF(pSpec);
            Py_XDECREF(pItf);
        }
    }
    if (PyErr_Occurred()) PyErr_Clear();
    return bActive;
}

static bool CheckMobileTouchScrollableAt(int mx, int my) {
    PyObject* pBuiltin = PyImport_AddModule("__builtin__");
    if (!pBuiltin || !PyObject_HasAttrString(pBuiltin, "_MobileBeginTouchScroll")) {
        if (PyErr_Occurred()) PyErr_Clear();
        return false;
    }
    PyObject* pRes = PyObject_CallMethod(pBuiltin, "_MobileBeginTouchScroll", "(ii)", mx, my);
    bool canScroll = (pRes && PyInt_Check(pRes) && PyInt_AsLong(pRes) != 0);
    Py_XDECREF(pRes);
    if (PyErr_Occurred()) PyErr_Clear();
    return canScroll;
}

static void UpdateMobileTouchScroll(int stepDy) {
    if (stepDy == 0) return;
    PyObject* pBuiltin = PyImport_AddModule("__builtin__");
    if (!pBuiltin || !PyObject_HasAttrString(pBuiltin, "_MobileUpdateTouchScroll")) {
        if (PyErr_Occurred()) PyErr_Clear();
        return;
    }
    PyObject* pRes = PyObject_CallMethod(pBuiltin, "_MobileUpdateTouchScroll", "(i)", stepDy);
    Py_XDECREF(pRes);
    if (PyErr_Occurred()) PyErr_Clear();
}

static UI::CWindow* FindHoldDraggableWindow(int x, int y) {
    if (IsMobileItemHoldOrAttachedActive()) {
        return nullptr;
    }
    UI::CWindowManager::Instance().RunMouseMove(x, y);
    UI::CWindow* pPointWnd = SanitizeButtonChildPickFlags(UI::CWindowManager::Instance().GetPointWindow());
    if (!pPointWnd) return nullptr;

    bool isThumb = false, isBtn = false;
    if (IsScrollOrSliderOrButtonWidget(pPointWnd, &isThumb, &isBtn)) {
        return nullptr;
    }

    UI::CWindow* pBestMovable = nullptr;
    bool bTouchedTitleBar = false;
    for (UI::CWindow* pCur = pPointWnd; pCur != nullptr; pCur = pCur->GetParent()) {
        const char* szName = pCur->GetName();
        if (szName) {
            if (strstr(szName, "Title") != nullptr || strstr(szName, "title") != nullptr) {
                bTouchedTitleBar = true;
            }
            if (strcmp(szName, "SliderBar") == 0 ||
                strcmp(szName, "ChatInputSet") == 0 ||
                strcmp(szName, "ChatWindow:ChatButton") == 0 ||
                strcmp(szName, "TaskBar") == 0 ||
                strcmp(szName, "MobileHUDFixed") == 0) {
                return nullptr;
            }
            if (strcmp(szName, "MobileChatCatcher") == 0) {
                return pCur;
            }
        }
        if (pCur->IsFlag(UI::CWindow::FLAG_MOVABLE) && !pCur->IsFlag(UI::CWindow::FLAG_RESTRICT_X)) {
            pBestMovable = pCur;
        }
    }

    if (!pBestMovable) return nullptr;

    const char* cBestName = pBestMovable->GetName();
    if (cBestName && strcmp(cBestName, "MobileEquipWindow") == 0) {
        return pBestMovable;
    }

    // For InventoryWindow (and SpecialStorageWindow / other windows), only allow dragging via the TitleBar!
    const RECT& rcBest = pBestMovable->GetRect();
    if (!bTouchedTitleBar && y > rcBest.top + 30) {
        return nullptr;
    }

    if (pBestMovable->GetWidth() >= g_AndroidScreenWidth - 32 &&
        pBestMovable->GetHeight() >= g_AndroidScreenHeight - 32) {
        return nullptr;
    }
    const char* cName = pBestMovable->GetName();
    if (cName && (strcmp(cName, "MobileHUDFixed") == 0 || strcmp(cName, "TaskBar") == 0 ||
                  strcmp(cName, "ChatWindow:ChatButton") == 0)) {
        return nullptr;
    }
    return pBestMovable;
}

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
static DWORD s_dwLastTappedTargetVID = 0;
static DWORD s_dwLastTappedTargetMs = 0;
static DWORD s_dwMobileAutoAttackVID = 0;
static bool  s_bAutoAttackMoveActive = false;
static float s_fAutoAttackMoveDeg = 0.0f;
static bool s_bPendingGroundLeftUp = false;
static int  s_iPendingGroundUpX = 0;
static int  s_iPendingGroundUpY = 0;

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
static jobject s_pLiveEditText = nullptr;
static std::u16string s_u16LastEditText;
static volatile bool s_bResetLiveEditTextPending = false;
static volatile DWORD s_dwLastTouchDownMs = 0;

static void EnsureAndroidLiveEditTextOnUiThread(JNIEnv* env) {
#if !defined(__APPLE__)
    if (!env) return;
    jclass mainActCls = env->FindClass("com/worldonline/client/MainActivity");
    if (!mainActCls) { env->ExceptionClear(); return; }

    if (!s_pLiveEditText) {
        jmethodID getEtMid = env->GetStaticMethodID(mainActCls, "getOrCreateLiveEditText", "()Landroid/widget/EditText;");
        jobject etObj = getEtMid ? env->CallStaticObjectMethod(mainActCls, getEtMid) : nullptr;
        env->ExceptionClear();
        if (etObj) {
            s_pLiveEditText = env->NewGlobalRef(etObj);
            s_u16LastEditText.assign(64, u' ');
            s_bResetLiveEditTextPending = false;
            env->DeleteLocalRef(etObj);
        }
    }

    if (s_pLiveEditText && (s_bResetLiveEditTextPending || !s_bSoftKeyboardVisible)) {
        s_bResetLiveEditTextPending = false;
        s_u16LastEditText.assign(64, u' ');
        jmethodID resetMid = env->GetStaticMethodID(mainActCls, "resetLiveEditTextOnUiThread", "()V");
        if (resetMid) env->CallStaticVoidMethod(mainActCls, resetMid);
        env->ExceptionClear();
    }
    env->DeleteLocalRef(mainActCls);
#endif
}

static void EnsureActiveEditLineFocusedBeforeIME() {
    PyRun_SimpleString(
        "try:\n"
        "    import constInfo\n"
        "    itf = constInfo.GetInterfaceInstance() if hasattr(constInfo, 'GetInterfaceInstance') else None\n"
        "    if itf and getattr(itf, 'wndChat', None) and itf.wndChat.IsEditMode():\n"
        "        cl = itf.wndChat.chatInputSet.chatLine\n"
        "        if not cl.IsFocus(): cl.SetFocus()\n"
        "except:\n"
        "    pass\n"
    );
}

static void SyncMobileLiveTypingToPython() {
    UI::CWindowManager::Instance().RunIMEUpdate();
    PyRun_SimpleString(
        "try:\n"
        "    import ime, ui, constInfo\n"
        "    itf = constInfo.GetInterfaceInstance() if hasattr(constInfo, 'GetInterfaceInstance') else None\n"
        "    if itf and getattr(itf, 'wndChat', None) and itf.wndChat.IsEditMode():\n"
        "        itf.wndChat.chatInputSet.chatLine.OnIMEUpdate()\n"
        "    if hasattr(ui, '_g_mobileKbBar') and ui._g_mobileKbBar and ui._g_mobileKbBar.IsShow():\n"
        "        ui._g_mobileKbBar.SetPreviewText(ime.GetText(0), 0)\n"
        "except:\n"
        "    pass\n"
    );
}

static void PasteUnicodeCharToMetin2IME(uint16_t ch) {
    if (ch < 32) return;
    EnsureActiveEditLineFocusedBeforeIME();

    // Handle single-key Arabic Lam-Alef ligatures (لا / لأ / لإ / لآ)
    if (ch >= 0xFEF5 && ch <= 0xFEFC) {
        unsigned int alefByte = 0xC7; // ا
        if (ch == 0xFEF5 || ch == 0xFEF6) alefByte = 0xC2;      // آ
        else if (ch == 0xFEF7 || ch == 0xFEF8) alefByte = 0xC3; // أ
        else if (ch == 0xFEF9 || ch == 0xFEFA) alefByte = 0xC5; // إ
        char cmd[96];
        snprintf(cmd, sizeof(cmd), "import ime\nime.PasteString('\\xe4\\x%02x')\n", alefByte);
        PyRun_SimpleString(cmd);
        return;
    }

    ch = (uint16_t)NormalizeArabicPresentationWChar((WCHAR)ch);

    // Convert Arabic-Indic digits (٠-٩ and ۰-۹) to standard ASCII '0'-'9'
    if (ch >= 0x0660 && ch <= 0x0669) ch = (uint16_t)('0' + (ch - 0x0660));
    else if (ch >= 0x06F0 && ch <= 0x06F9) ch = (uint16_t)('0' + (ch - 0x06F0));

    unsigned char outCh = 0;
    if (ch < 127) {
        outCh = (unsigned char)ch;
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
            if (s_cp1256Map[idx] == ch) {
                outCh = (unsigned char)(0x80 + idx);
                break;
            }
        }
    }
    if (outCh != 0) {
        char cmd[64];
        snprintf(cmd, sizeof(cmd), "import ime\nime.PasteString('\\x%02x')\n", (unsigned int)outCh);
        PyRun_SimpleString(cmd);
    }
}

void AndroidToggleSoftKeyboard(bool show);

static bool PollAndroidLiveEditText(JNIEnv* env) {
#if !defined(__APPLE__)
    if (!env || !s_bSoftKeyboardVisible || !s_pLiveEditText)
        return false;

    static jclass s_mainActCls = nullptr;
    static jmethodID s_isResettingMid = nullptr;
    static jmethodID s_getTextMid = nullptr;
    static jmethodID s_toStringMid = nullptr;
    if (!s_getTextMid) {
        jclass localMain = env->FindClass("com/worldonline/client/MainActivity");
        if (localMain) {
            s_mainActCls = (jclass)env->NewGlobalRef(localMain);
            s_isResettingMid = env->GetStaticMethodID(s_mainActCls, "isResettingLiveEditText", "()Z");
            env->DeleteLocalRef(localMain);
        }
        jclass tvCls = env->FindClass("android/widget/TextView");
        s_getTextMid = env->GetMethodID(tvCls, "getText", "()Ljava/lang/CharSequence;");
        env->DeleteLocalRef(tvCls);
        jclass objCls = env->FindClass("java/lang/Object");
        s_toStringMid = env->GetMethodID(objCls, "toString", "()Ljava/lang/String;");
        env->DeleteLocalRef(objCls);
        env->ExceptionClear();
    }
    if (!s_getTextMid || !s_toStringMid)
        return false;
    if (s_mainActCls && s_isResettingMid && env->CallStaticBooleanMethod(s_mainActCls, s_isResettingMid) == JNI_TRUE) {
        env->ExceptionClear();
        return false;
    }

    jobject csObj = env->CallObjectMethod(s_pLiveEditText, s_getTextMid);
    if (env->ExceptionCheck() || !csObj) {
        env->ExceptionClear();
        return false;
    }
    jstring jstr = (jstring)env->CallObjectMethod(csObj, s_toStringMid);
    env->DeleteLocalRef(csObj);
    if (env->ExceptionCheck() || !jstr) {
        env->ExceptionClear();
        return false;
    }

    jsize len = env->GetStringLength(jstr);
    const jchar* chars = env->GetStringChars(jstr, nullptr);
    std::u16string rawStr;
    if (chars && len > 0) {
        rawStr.assign(chars, chars + len);
    }
    if (chars) {
        env->ReleaseStringChars(jstr, chars);
    }
    env->DeleteLocalRef(jstr);

    if (rawStr == s_u16LastEditText)
        return false;

    bool pressedEnter = false;
    std::u16string cleanStr;
    cleanStr.reserve(rawStr.size());
    for (char16_t c : rawStr) {
        if (c == u'\n' || c == u'\r') {
            pressedEnter = true;
        } else {
            cleanStr.push_back(c);
        }
    }

    if (cleanStr.size() > 64 && cleanStr[0] != u' ' && s_u16LastEditText.size() >= 64 && s_u16LastEditText.substr(0, 64) == std::u16string(64, u' ')) {
        size_t trailingSpaces = 0;
        while (trailingSpaces < cleanStr.size() && cleanStr[cleanStr.size() - 1 - trailingSpaces] == u' ') {
            ++trailingSpaces;
        }
        if (trailingSpaces >= 64) {
            std::u16string typedPart = cleanStr.substr(0, cleanStr.size() - 64);
            cleanStr = std::u16string(64, u' ') + typedPart;
        }
    }

    size_t prefixLen = 0;
    size_t minLen = std::min(s_u16LastEditText.size(), cleanStr.size());
    while (prefixLen < minLen && s_u16LastEditText[prefixLen] == cleanStr[prefixLen]) {
        ++prefixLen;
    }

    EnsureActiveEditLineFocusedBeforeIME();
    size_t numBackspaces = s_u16LastEditText.size() - prefixLen;
    for (size_t i = 0; i < numBackspaces; ++i) {
        PyRun_SimpleString("import ime\nime.PasteBackspace()\n");
    }
    for (size_t i = prefixLen; i < cleanStr.size(); ++i) {
        PasteUnicodeCharToMetin2IME((uint16_t)cleanStr[i]);
    }

    s_u16LastEditText = cleanStr;
    // Immediately refill the 64 spaces on the UI thread when holding Delete so continuous backspace never stops!
    if (cleanStr.size() < 16 || cleanStr.size() > 200) {
        s_u16LastEditText.assign(64, u' ');
        jclass mainActCls = env->FindClass("com/worldonline/client/MainActivity");
        if (mainActCls) {
            jmethodID resetMid = env->GetStaticMethodID(mainActCls, "resetLiveEditTextOnUiThread", "()V");
            if (resetMid) env->CallStaticVoidMethod(mainActCls, resetMid);
            env->ExceptionClear();
            env->DeleteLocalRef(mainActCls);
        }
    }

    SyncMobileLiveTypingToPython();

    if (pressedEnter) {
        s_bResetLiveEditTextPending = true;
        UI::CWindow* pBefore = UI::CWindowManager::Instance().GetActivateWindow();
        UI::CWindowManager::Instance().RunIMEReturnEvent();
        UI::CWindow* pAfter = UI::CWindowManager::Instance().GetActivateWindow();
        if (!pAfter || pAfter == pBefore) {
            AndroidToggleSoftKeyboard(false);
        }
    }
    return true;
#else
    return false;
#endif
}

void AndroidToggleSoftKeyboard(bool show) {
    if (show) {
        DWORD nowMs = ELTimer_GetMSec();
        if (!s_bExplicitKeyboardRequest) {
            if (s_bLastTouchSnappedToButton || s_dwLastTouchDownMs == 0 || (nowMs - s_dwLastTouchDownMs > 450)) {
                return;
            }
            UI::CWindow* pAct = UI::CWindowManager::InstancePtr() ? UI::CWindowManager::Instance().GetActivateWindow() : nullptr;
            if (!pAct || !pAct->IsShow() || pAct->IsFlag(UI::CWindow::FLAG_NOT_PICK) || dynamic_cast<UI::CTextLine*>(pAct) == nullptr) {
                return;
            }
            // Never open keyboard if pAct is a child of a CButton (such as ToolTipText or ButtonText)
            for (UI::CWindow* pWalk = pAct->GetParent(); pWalk != nullptr; pWalk = pWalk->GetParent()) {
                if (dynamic_cast<UI::CButton*>(pWalk) != nullptr) {
                    pAct->AddFlag(UI::CWindow::FLAG_NOT_PICK);
                    return;
                }
            }
            const RECT& rc = pAct->GetRect();
            if (s_iLastTouchDownX < rc.left - 14 || s_iLastTouchDownX > rc.right + 14 ||
                s_iLastTouchDownY < rc.top - 14  || s_iLastTouchDownY > rc.bottom + 14) {
                return;
            }
        }
        s_bExplicitKeyboardRequest = false;
        s_bResetLiveEditTextPending = true;
        s_dwLastKeyboardOpenMs = nowMs;
    } else {
        s_bExplicitKeyboardRequest = false;
        s_bResetLiveEditTextPending = true;
    }
    s_bSoftKeyboardVisible = show;

    // Dynamic ThinBoard TOP_MOST preview with larger 18px bold font
    PyRun_SimpleString(show ?
        "try:\n"
        "    import ui, wndMgr, localeInfo\n"
        "    if not hasattr(ui, '_g_mobileKbBar') or ui._g_mobileKbBar is None:\n"
        "        class _MobileKbPreview(ui.ThinBoard):\n"
        "            def __init__(self):\n"
        "                ui.ThinBoard.__init__(self, 'TOP_MOST')\n"
        "                self.AddFlag('not_pick')\n"
        "                self.txt = ui.TextLine()\n"
        "                self.txt.SetParent(self)\n"
        "                try:\n"
        "                    baseFnt = getattr(localeInfo, 'UI_DEF_FONT', 'Tahoma:12').split(':')[0]\n"
        "                    self.txt.SetFontName(baseFnt + ':18b')\n"
        "                except:\n"
        "                    pass\n"
        "                self.txt.SetHorizontalAlignCenter()\n"
        "                self.txt.SetVerticalAlignCenter()\n"
        "                self.txt.SetOutline()\n"
        "                self.txt.SetPackedFontColor(0xFFFFFFFF)\n"
        "                self.txt.AddFlag('not_pick')\n"
        "                self.txt.Show()\n"
        "                self.SetPreviewText('', 0)\n"
        "            def SetPreviewText(self, val, isSecret):\n"
        "                disp = ('*' * len(val)) if isSecret else (val if val else '')\n"
        "                self.txt.SetText(disp)\n"
        "                tw, th = self.txt.GetTextSize()\n"
        "                bw = max(96, tw + 36)\n"
        "                bh = max(38, th + 18)\n"
        "                sw = wndMgr.GetScreenWidth()\n"
        "                self.SetSize(bw, bh)\n"
        "                self.SetPosition((sw - bw) / 2, 10)\n"
        "                self.txt.SetPosition(bw / 2, bh / 2)\n"
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
    InstallIosInputHooks();
    IOSToggleSoftKeyboard(show);
    return;
#else
    if (!g_pJavaVM) return;
    JNIEnv* env = nullptr;
    if (g_pJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK || !env) return;

    EnsureAndroidLiveEditTextOnUiThread(env);
    jclass mainActCls = env->FindClass("com/worldonline/client/MainActivity");
    if (mainActCls) {
        jmethodID setKbMid = env->GetStaticMethodID(mainActCls, "setSoftKeyboardVisibleOnUiThread", "(Z)V");
        if (setKbMid) {
            env->CallStaticVoidMethod(mainActCls, setKbMid, (jboolean)(show ? JNI_TRUE : JNI_FALSE));
        }
        env->ExceptionClear();
        env->DeleteLocalRef(mainActCls);
    }
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

static bool IsPointOnUILayer(int x, int y);

static void ReleasePointerSlot(int idx, int x, int y, int maskedAction) {
    if (idx < 0 || idx >= 4) return;
    SMobilePointerSlot& slot = s_pointers[idx];
    if (!slot.active) return;

    int rx = (x >= 0) ? x : slot.curX;
    int ry = (y >= 0) ? y : slot.curY;

    if (slot.role == TOUCH_ROLE_JOYSTICK) {
        ResetJoystickState();
    } else if (slot.role == TOUCH_ROLE_UI) {
        bool wasDraggingWnd = slot.isDraggingWindow;
        bool wasTouchScrolling = slot.isTouchScrolling;
        UI::CWindow* pDraggedWnd = slot.pHoldDragWnd;
        slot.active = false;
        slot.role = TOUCH_ROLE_NONE;
        slot.isDraggingCamera = false;
        slot.isDraggingWindow = false;
        slot.holdDragCancelled = false;
        slot.canTouchScroll = false;
        slot.isTouchScrolling = false;
        slot.pHoldDragWnd = nullptr;
        UpdateUiTouchDownFlag();
        if (wasTouchScrolling) {
            return;
        }
        if (wasDraggingWnd) {
            if (pDraggedWnd && pDraggedWnd->GetName() &&
                (strcmp(pDraggedWnd->GetName(), "ChatWindow") == 0 || strcmp(pDraggedWnd->GetName(), "MobileChatCatcher") == 0)) {
                long cx = 0, cy = 0;
                pDraggedWnd->GetPosition(&cx, &cy);
                char cmd[240];
                snprintf(cmd, sizeof(cmd),
                    "try:\n"
                    "    import constInfo\n"
                    "    if hasattr(constInfo, 'OnMobileChatDragged'): constInfo.OnMobileChatDragged(%ld, %ld, '%s')\n"
                    "except:\n"
                    "    pass\n", cx, cy, pDraggedWnd->GetName());
                PyRun_SimpleString(cmd);
            }
            return;
        }
        int mx = rx, my = ry;
        // If ACTION_DOWN snapped onto a button and the finger didn't drag far, release squarely on the snapped button!
        int totalMoveSq = (rx - slot.startX) * (rx - slot.startX) + (ry - slot.startY) * (ry - slot.startY);
        if (s_bLastTouchSnappedToButton && totalMoveSq <= 576) {
            mx = s_iLastTouchDownX;
            my = s_iLastTouchDownY;
        } else {
            MapTouchThroughScaledWindows(rx, ry, &mx, &my);
        }
        g_AndroidMouseX = mx;
        g_AndroidMouseY = my;
        UI::CWindowManager::Instance().RunMouseMove(mx, my);
        UI::CWindowManager::Instance().RunMouseLeftButtonUp(mx, my);

        // Only auto-drop on release if the finger dragged across to a different slot (> 35px) or onto the 3D ground.
        // If the user pressed-and-held in place (<= 35px) and lifted their finger, keep the ghost attached!
        int dragDistSq = (rx - slot.startX) * (rx - slot.startX) + (ry - slot.startY) * (ry - slot.startY);
        if (dragDistSq > 1225) {
            PyObject* pMm = PyImport_AddModule("mouseModule");
            PyObject* pMc = pMm ? PyObject_GetAttrString(pMm, "mouseController") : nullptr;
            PyObject* pIsAtt = pMc ? PyObject_CallMethod(pMc, "isAttached", NULL) : nullptr;
            bool bAttached = (pIsAtt && PyObject_IsTrue(pIsAtt));
            Py_XDECREF(pIsAtt);
            Py_XDECREF(pMc);
            if (PyErr_Occurred()) PyErr_Clear();

            if (bAttached) {
                UI::CWindowManager::Instance().RunMouseLeftButtonDown(mx, my);
                UI::CWindowManager::Instance().RunMouseLeftButtonUp(mx, my);
                if (!IsPointOnUILayer(mx, my)) {
                    PyRun_SimpleString(
                        "try:\n"
                        "    import mouseModule\n"
                        "    if mouseModule.mouseController.isAttached():\n"
                        "        mouseModule.mouseController.DeattachObject()\n"
                        "except:\n"
                        "    pass\n"
                    );
                }
            }
        }
        return;
    } else if (slot.role == TOUCH_ROLE_WORLD) {
        DWORD heldMs = ELTimer_GetMSec() - slot.downTimeMs;
        int totalDx = rx - slot.startX;
        int totalDy = ry - slot.startY;
        if (maskedAction != 3 && !slot.isDraggingCamera && heldMs < 480 && (totalDx * totalDx + totalDy * totalDy <= 900)) {
            // If an item or skill is attached to mouseController, tapping the 3D ground drops the item or detaches the skill!
            PyObject* pMm = PyImport_AddModule("mouseModule");
            PyObject* pMc = pMm ? PyObject_GetAttrString(pMm, "mouseController") : nullptr;
            PyObject* pIsAtt = pMc ? PyObject_CallMethod(pMc, "isAttached", NULL) : nullptr;
            bool bAttachedOnCursor = (pIsAtt && PyObject_IsTrue(pIsAtt));
            Py_XDECREF(pIsAtt);
            Py_XDECREF(pMc);
            if (PyErr_Occurred()) PyErr_Clear();

            DWORD pickedVID = bAttachedOnCursor ? 0 : Pick3DActorAtScreenPos(rx, ry);
            if (pickedVID != 0 && CPythonPlayer::InstancePtr()) {
                DWORD nowTapMs = ELTimer_GetMSec();
                CAndroidPlayerAccessor& rkPlayer = static_cast<CAndroidPlayerAccessor&>(CPythonPlayer::Instance());
                DWORD prevTargetVID = rkPlayer.GetTargetVID();
                bool isSecondTapOnSameTarget = (pickedVID == prevTargetVID ||
                    (pickedVID == s_dwLastTappedTargetVID && (nowTapMs - s_dwLastTappedTargetMs) < 4000));

                s_dwLastTappedTargetVID = pickedVID;
                s_dwLastTappedTargetMs = nowTapMs;
                rkPlayer.SetTarget(pickedVID, TRUE);

                CInstanceBase* pkInst = CPythonCharacterManager::InstancePtr()
                    ? CPythonCharacterManager::Instance().GetInstancePtr(pickedVID) : nullptr;
                if (pkInst) {
                    if (pkInst->IsNPC() || pkInst->IsShop()) {
                        s_dwMobileAutoAttackVID = 0;
                        rkPlayer.SetAttackKeyState(false);
                        char cmd[128];
                        snprintf(cmd, sizeof(cmd),
                            "try:\n"
                            "    import net\n"
                            "    net.SendOnClickPacket(%u)\n"
                            "except:\n"
                            "    pass\n", (unsigned int)pickedVID);
                        PyRun_SimpleString(cmd);
                    } else if (pkInst->IsPC()) {
                        if (rkPlayer.m_ppyGameWindow) {
                            PyCallClassMemberFunc(rkPlayer.m_ppyGameWindow, "SetPCTargetBoard",
                                                  Py_BuildValue("(is)", (int)pickedVID, pkInst->GetNameString()));
                        }
                        if (isSecondTapOnSameTarget && pkInst->IsEnemy()) {
                            s_dwMobileAutoAttackVID = pickedVID;
                        } else {
                            s_dwMobileAutoAttackVID = 0;
                        }
                    } else {
                        if (isSecondTapOnSameTarget) {
                            s_dwMobileAutoAttackVID = pickedVID;
                        } else {
                            s_dwMobileAutoAttackVID = 0;
                            rkPlayer.SetAttackKeyState(false);
                        }
                    }
                }
            } else {
                if (s_dwMobileAutoAttackVID != 0 && CPythonPlayer::InstancePtr()) {
                    s_dwMobileAutoAttackVID = 0;
                    s_bAutoAttackMoveActive = false;
                    CPythonPlayer::Instance().SetAttackKeyState(false);
                }
                g_AndroidMouseX = rx;
                g_AndroidMouseY = ry;
                UI::CWindowManager::Instance().RunMouseMove(rx, ry);
                UI::CWindowManager::Instance().RunMouseLeftButtonDown(rx, ry);
                if (bAttachedOnCursor) {
                    UI::CWindowManager::Instance().RunMouseLeftButtonUp(rx, ry);
                    PyRun_SimpleString(
                        "try:\n"
                        "    import mouseModule, constInfo\n"
                        "    if mouseModule.mouseController.isAttached():\n"
                        "        mouseModule.mouseController.DeattachObject()\n"
                        "    itf = constInfo.GetInterfaceInstance() if hasattr(constInfo, 'GetInterfaceInstance') else None\n"
                        "    if itf and getattr(itf, 'tooltipItem', None):\n"
                        "        itf.tooltipItem.HideToolTip()\n"
                        "except:\n"
                        "    pass\n"
                    );
                } else {
                    // Defer LeftButtonUp until right after Loop() so CPythonPlayer::Update() processes MBS_PRESS raycast first!
                    s_bPendingGroundLeftUp = true;
                    s_iPendingGroundUpX = rx;
                    s_iPendingGroundUpY = ry;
                }
            }
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

static PyObject* mobileInput_GetCameraTouchState(PyObject* /*self*/, PyObject* /*args*/) {
    int wIdx0 = -1, wIdx1 = -1;
    int worldCount = CountActiveWorldPointers(&wIdx0, &wIdx1);
    int draggingCam = IsAnyPointerDraggingCamera() ? 1 : 0;
    int x0 = (wIdx0 >= 0) ? s_pointers[wIdx0].curX : -1;
    int y0 = (wIdx0 >= 0) ? s_pointers[wIdx0].curY : -1;
    int x1 = (wIdx1 >= 0) ? s_pointers[wIdx1].curX : -1;
    int y1 = (wIdx1 >= 0) ? s_pointers[wIdx1].curY : -1;
    int sx0 = (wIdx0 >= 0) ? s_pointers[wIdx0].startX : -1;
    int sy0 = (wIdx0 >= 0) ? s_pointers[wIdx0].startY : -1;
    return Py_BuildValue("(iiiiiiiifii)", worldCount, draggingCam, x0, y0, x1, y1, sx0, sy0, s_fLastPinchSpan, g_AndroidRawTouchX, g_AndroidRawTouchY);
}

extern "C" bool AndroidGetJoystickMoveDir(float* outDeg) {
    if (s_bJoyActive) {
        float mag = hypotf(s_fJoyNormX, s_fJoyNormY);
        if (mag <= 0.15f) return false;
        if (outDeg) {
            float vx = s_fJoyNormX / mag;
            float vy = s_fJoyNormY / mag;
            float dot = -vy;
            if (dot < -1.0f) dot = -1.0f;
            else if (dot > 1.0f) dot = 1.0f;
            float ret = acosf(dot) * (180.0f / 3.14159265f);
            if (vx < 0.0f) ret = 360.0f - ret;
            *outDeg = 360.0f - ret;
        }
        return true;
    }
    if (s_bAutoAttackMoveActive && s_dwMobileAutoAttackVID != 0) {
        if (outDeg) *outDeg = s_fAutoAttackMoveDeg;
        return true;
    }
    return false;
}

static bool IsScreenPointCoveredByOpenWindow(int rawX, int rawY) {
    int mx = rawX, my = rawY;
    MapTouchThroughScaledWindows(rawX, rawY, &mx, &my);
    UI::CWindow* pRoot = CAndroidWindowHierarchyAccessor::EnsureRootWindow();
    UI::CWindow* pWnd = pRoot ? CAndroidWindowHierarchyAccessor::PickWindowAtReadOnly(pRoot, mx, my) : nullptr;
    if (!pWnd) return false;
    bool isHud = false;
    bool isUiOrTop = false;
    for (UI::CWindow* pCur = pWnd; pCur != nullptr; pCur = pCur->GetParent()) {
        const char* szName = pCur->GetName();
        if (!szName) continue;
        if (strcmp(szName, "MobileHUDFixed") == 0) {
            isHud = true;
            break;
        }
        if (strcmp(szName, "UI") == 0 || strcmp(szName, "TOP_MOST") == 0) {
            isUiOrTop = true;
        }
    }
    return (!isHud && isUiOrTop);
}

static PyObject* mobileInput_IsRectTouchHeld(PyObject* self, PyObject* args) {
    int rx = 0, ry = 0, rw = 0, rh = 0;
    if (!PyArg_ParseTuple(args, "iiii", &rx, &ry, &rw, &rh))
        return Py_BuildValue("i", 0);
    for (int i = 0; i < 4; ++i) {
        if (!s_pointers[i].active || s_pointers[i].role == TOUCH_ROLE_JOYSTICK || s_pointers[i].isDraggingWindow)
            continue;
        int sx = s_pointers[i].startX, sy = s_pointers[i].startY;
        int cx = s_pointers[i].curX,   cy = s_pointers[i].curY;
        bool inStart = (sx >= rx && sx < (rx + rw) && sy >= ry && sy < (ry + rh));
        bool inCur   = (cx >= rx && cx < (rx + rw) && cy >= ry && cy < (ry + rh));
        if (inStart || inCur) {
            // Never trigger HUD action buttons when an open window (Inventory, Equipment, SpecialStorage, Tooltip, Quest) covers the touch!
            if (IsScreenPointCoveredByOpenWindow(sx, sy) || IsScreenPointCoveredByOpenWindow(cx, cy))
                continue;
            return Py_BuildValue("i", 1);
        }
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
    if (show != 0) s_bExplicitKeyboardRequest = true;
    AndroidToggleSoftKeyboard(show != 0);
    return Py_BuildValue("i", 1);
}

static PyObject* mobileInput_OpenURL(PyObject* self, PyObject* args) {
    const char* url = nullptr;
    if (!PyArg_ParseTuple(args, "s", &url) || !url || !*url)
        return Py_BuildValue("i", 0);
#if defined(__APPLE__)
    IOSOpenURL(url);
#else
    if (g_pJavaVM) {
        JNIEnv* env = nullptr;
        if (g_pJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_OK && env) {
            jclass atCls = env->FindClass("android/app/ActivityThread");
            if (atCls) {
                jmethodID curAppMid = env->GetStaticMethodID(atCls, "currentApplication", "()Landroid/app/Application;");
                jobject appObj = curAppMid ? env->CallStaticObjectMethod(atCls, curAppMid) : nullptr;
                env->ExceptionClear();
                if (appObj) {
                    jclass uriCls = env->FindClass("android/net/Uri");
                    jclass intentCls = env->FindClass("android/content/Intent");
                    if (uriCls && intentCls) {
                        jmethodID parseMid = env->GetStaticMethodID(uriCls, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
                        jstring jurl = env->NewStringUTF(url);
                        jobject uriObj = parseMid ? env->CallStaticObjectMethod(uriCls, parseMid, jurl) : nullptr;
                        env->ExceptionClear();
                        if (uriObj) {
                            jstring jact = env->NewStringUTF("android.intent.action.VIEW");
                            jmethodID intentCtor = env->GetMethodID(intentCls, "<init>", "(Ljava/lang/String;Landroid/net/Uri;)V");
                            jobject intentObj = intentCtor ? env->NewObject(intentCls, intentCtor, jact, uriObj) : nullptr;
                            env->ExceptionClear();
                            if (intentObj) {
                                jmethodID addFlagsMid = env->GetMethodID(intentCls, "addFlags", "(I)Landroid/content/Intent;");
                                if (addFlagsMid) env->CallObjectMethod(intentObj, addFlagsMid, (jint)0x10000000); // FLAG_ACTIVITY_NEW_TASK
                                jclass ctxCls = env->GetObjectClass(appObj);
                                jmethodID startActMid = env->GetMethodID(ctxCls, "startActivity", "(Landroid/content/Intent;)V");
                                if (startActMid) env->CallVoidMethod(appObj, startActMid, intentObj);
                                env->ExceptionClear();
                                env->DeleteLocalRef(ctxCls);
                                env->DeleteLocalRef(intentObj);
                            }
                            env->DeleteLocalRef(jact);
                            env->DeleteLocalRef(uriObj);
                        }
                        env->DeleteLocalRef(jurl);
                    }
                    if (uriCls) env->DeleteLocalRef(uriCls);
                    if (intentCls) env->DeleteLocalRef(intentCls);
                    env->DeleteLocalRef(appObj);
                }
                env->DeleteLocalRef(atCls);
            }
        }
    }
#endif
    return Py_BuildValue("i", 1);
}

static PyObject* mobileInput_CloseApp(PyObject* /*self*/, PyObject* /*args*/) {
    s_bEngineReady = false;
    signal(SIGSEGV, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    unlink("crash_report.txt");
#if defined(__APPLE__)
    IOSCloseApp();
#else
    if (g_pJavaVM) {
        JNIEnv* env = nullptr;
        if (g_pJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_OK && env) {
            if (s_pLiveEditText) {
                jclass viewCls = env->FindClass("android/view/View");
                jmethodID getCtxMid = viewCls ? env->GetMethodID(viewCls, "getContext", "()Landroid/content/Context;") : nullptr;
                jobject actObj = getCtxMid ? env->CallObjectMethod(s_pLiveEditText, getCtxMid) : nullptr;
                env->ExceptionClear();
                if (actObj) {
                    jclass actCls = env->FindClass("android/app/Activity");
                    if (actCls && env->IsInstanceOf(actObj, actCls)) {
                        jmethodID moveBackMid = env->GetMethodID(actCls, "moveTaskToBack", "(Z)Z");
                        if (moveBackMid) env->CallBooleanMethod(actObj, moveBackMid, (jboolean)JNI_TRUE);
                        env->ExceptionClear();
                        jmethodID finishMid = env->GetMethodID(actCls, "finishAndRemoveTask", "()V");
                        env->ExceptionClear();
                        if (!finishMid) finishMid = env->GetMethodID(actCls, "finish", "()V");
                        if (finishMid) env->CallVoidMethod(actObj, finishMid);
                        env->ExceptionClear();
                    }
                    if (actCls) env->DeleteLocalRef(actCls);
                    env->DeleteLocalRef(actObj);
                }
                if (viewCls) env->DeleteLocalRef(viewCls);
            }
        }
    }
#endif
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

//#define ENABLE_MOBILE_PERF_DEBUGGER // <-- Comment out to disable the profiler

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

static void EnsureParentDirsExistForPath(const std::string& filePath) {
    size_t pos = 0;
    while ((pos = filePath.find('/', pos + 1)) != std::string::npos) {
        std::string dir = filePath.substr(0, pos);
        if (!dir.empty()) mkdir(dir.c_str(), 0777);
    }
}

static PyObject* mobileInput_ReadPackFile(PyObject* /*self*/, PyObject* args) {
    const char* c_szPath = nullptr;
    if (!PyArg_ParseTuple(args, "s", &c_szPath) || !c_szPath || !*c_szPath)
        return Py_BuildValue("s", "");

    std::string normPath = c_szPath;
    for (char& c : normPath) if (c == '\\') c = '/';
    std::string lowerPath = normPath;
    for (char& c : lowerPath) c = (char)tolower((unsigned char)c);

    CMappedFile file;
    LPCVOID pData = nullptr;
    std::string dPath = "d:/ymir work/" + lowerPath;
    if (CEterPackManager::InstancePtr() &&
        (CEterPackManager::Instance().Get(file, normPath.c_str(), &pData) ||
         CEterPackManager::Instance().Get(file, lowerPath.c_str(), &pData) ||
         CEterPackManager::Instance().Get(file, dPath.c_str(), &pData))) {
        if (pData && file.Size() > 0) {
            // Also cache on local disk so C++ fopen() calls (like npcLocationHelper) can open it directly
            if (access(lowerPath.c_str(), F_OK) != 0) {
                EnsureParentDirsExistForPath(lowerPath);
                if (FILE* fp = fopen(lowerPath.c_str(), "wb")) {
                    fwrite(pData, 1, file.Size(), fp);
                    fclose(fp);
                }
            }
            return PyString_FromStringAndSize((const char*)pData, (Py_ssize_t)file.Size());
        }
    }
    return Py_BuildValue("s", "");
}

static uintptr_t ExtractMobileWindowKey(PyObject* pyHandle) {
    if (!pyHandle || pyHandle == Py_None) return 0;
    uintptr_t raw = (uintptr_t)PyLong_AsUnsignedLongLongMask(pyHandle);
    if (PyErr_Occurred()) {
        PyErr_Clear();
        return (uintptr_t)pyHandle;
    }
    return raw;
}

static PyObject* mobileInput_SetWindowScale(PyObject* /*self*/, PyObject* args) {
    PyObject* pyHandle = nullptr;
    int gx = 0, gy = 0, w = 0, h = 0;
    float scale = 1.0f;

    if (PyTuple_Size(args) >= 6) {
        if (!PyArg_ParseTuple(args, "Oiiiif", &pyHandle, &gx, &gy, &w, &h, &scale)) {
            PyErr_Clear();
            return Py_BuildValue("i", 0);
        }
        uintptr_t key = ExtractMobileWindowKey(pyHandle);
        if (key == 0)
            return Py_BuildValue("i", 0);
        if (fabsf(scale - 1.0f) <= 0.01f || w <= 0 || h <= 0) {
            s_scaledWindows.erase(key);
            return Py_BuildValue("i", 1);
        }
        SScaledWindowRect& entry = s_scaledWindows[key];
        entry.pWnd = nullptr;
        entry.scale = scale;
        entry.x = gx;
        entry.y = gy;
        entry.w = w;
        entry.h = h;
        entry.lastSeenFrame = s_uMobileRenderFrame;
        return Py_BuildValue("i", 1);
    }

    if (!PyArg_ParseTuple(args, "Of", &pyHandle, &scale)) {
        PyErr_Clear();
        return Py_BuildValue("i", 0);
    }
    uintptr_t key = ExtractMobileWindowKey(pyHandle);
    if (key == 0)
        return Py_BuildValue("i", 0);

    if (fabsf(scale - 1.0f) <= 0.01f) {
        s_scaledWindows.erase(key);
        return Py_BuildValue("i", 1);
    }

    PyObject* pWndMgr = PyImport_AddModule("wndMgr");
    if (pWndMgr) {
        PyObject* pShow = PyObject_CallMethod(pWndMgr, "IsShow", "(O)", pyHandle);
        bool bShow = (pShow && PyInt_Check(pShow) && PyInt_AsLong(pShow) != 0);
        Py_XDECREF(pShow);
        if (!bShow) {
            if (PyErr_Occurred()) PyErr_Clear();
            s_scaledWindows.erase(key);
            return Py_BuildValue("i", 1);
        }
        PyObject* pPos = PyObject_CallMethod(pWndMgr, "GetGlobalPosition", "(O)", pyHandle);
        PyObject* pW   = PyObject_CallMethod(pWndMgr, "GetWindowWidth", "(O)", pyHandle);
        PyObject* pH   = PyObject_CallMethod(pWndMgr, "GetWindowHeight", "(O)", pyHandle);
        if (pPos && PyTuple_Check(pPos) && PyTuple_Size(pPos) >= 2) {
            gx = (int)PyInt_AsLong(PyTuple_GetItem(pPos, 0));
            gy = (int)PyInt_AsLong(PyTuple_GetItem(pPos, 1));
        }
        if (pW && PyInt_Check(pW)) w = (int)PyInt_AsLong(pW);
        if (pH && PyInt_Check(pH)) h = (int)PyInt_AsLong(pH);
        Py_XDECREF(pPos);
        Py_XDECREF(pW);
        Py_XDECREF(pH);
        if (PyErr_Occurred()) PyErr_Clear();

        if (w > 0 && h > 0) {
            SScaledWindowRect& entry = s_scaledWindows[key];
            entry.pWnd = nullptr;
            entry.scale = scale;
            entry.x = gx;
            entry.y = gy;
            entry.w = w;
            entry.h = h;
            entry.lastSeenFrame = s_uMobileRenderFrame;
        }
    }
    return Py_BuildValue("i", 1);
}

static PyObject* mobileInput_BeginWindowScale(PyObject* /*self*/, PyObject* args) {
    int gx = 0, gy = 0;
    float scale = 1.0f;
    if (!PyArg_ParseTuple(args, "iif", &gx, &gy, &scale))
        return Py_BuildValue("i", 0);
    if (fabsf(scale - 1.0f) > 0.01f && CStateManager::InstancePtr()) {
        CGraphicTextInstance::FlushBatchRender();
        if (!s_bUiScaleActive) {
            CStateManager::Instance().GetTransform(D3DTS_VIEW, &s_savedUiViewMat);
            s_bUiScaleActive = true;
        }
        D3DXMATRIX mat;
        D3DXMatrixIdentity(&mat);
        mat._11 = scale;
        mat._22 = scale;
        mat._41 = floorf((float)gx * (1.0f - scale) + 0.5f);
        mat._42 = floorf((float)gy * (1.0f - scale) + 0.5f);
        CStateManager::Instance().SetTransform(D3DTS_VIEW, &mat);
    }
    return Py_BuildValue("i", 1);
}

static PyObject* mobileInput_EndWindowScale(PyObject* /*self*/, PyObject* /*args*/) {
    if (s_bUiScaleActive && CStateManager::InstancePtr()) {
        CGraphicTextInstance::FlushBatchRender();
        CStateManager::Instance().SetTransform(D3DTS_VIEW, &s_savedUiViewMat);
        s_bUiScaleActive = false;
    }
    return Py_BuildValue("i", 1);
}

static PyMethodDef s_mobileInputMethods[] = {
    { "ReadPackFile",        mobileInput_ReadPackFile,        METH_VARARGS, "Read file from EterPack and cache on disk" },
    { "SetWindowScale",      mobileInput_SetWindowScale,      METH_VARARGS, "Register scaled UI window" },
    { "BeginWindowScale",    mobileInput_BeginWindowScale,    METH_VARARGS, "Begin 2D UI scale matrix" },
    { "EndWindowScale",      mobileInput_EndWindowScale,      METH_VARARGS, "End 2D UI scale matrix" },
    { "SetJoystickZone",     mobileInput_SetJoystickZone,     METH_VARARGS, "Set virtual joystick zone" },
    { "GetJoystickState",    mobileInput_GetJoystickState,    METH_VARARGS, "Get joystick state tuple" },
    { "IsUiTouchDown",       mobileInput_IsUiTouchDown,       METH_VARARGS, "Check if UI touch is held" },
    { "GetCameraTouchState", mobileInput_GetCameraTouchState, METH_VARARGS, "Get 3D camera drag and pinch zoom touch state" },
    { "IsRectTouchHeld",     mobileInput_IsRectTouchHeld,     METH_VARARGS, "Check if any touch started in rect" },
    { "GetScreenSize",       mobileInput_GetScreenSize,       METH_VARARGS, "Get native Android screen size" },
    { "ShowKeyboard",        mobileInput_ShowKeyboard,        METH_VARARGS, "Show or hide Android soft keyboard" },
    { "OpenURL",             mobileInput_OpenURL,             METH_VARARGS, "Open external URL in mobile browser" },
    { "CloseApp",            mobileInput_CloseApp,            METH_VARARGS, "Finish Android Activity cleanly" },
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

JNIEXPORT void JNICALL Java_com_worldonline_client_NativeLib_render(JNIEnv* env, jobject obj) {
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
            if (ev.x >= 0 && ev.y >= 0) {
                g_AndroidRawTouchX = ev.x;
                g_AndroidRawTouchY = ev.y;
            }

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

            #ifdef __APPLE__
            InstallIosInputHooks();
#endif
            if (maskedAction == 0 || maskedAction == 5) { // ACTION_DOWN or ACTION_POINTER_DOWN
                // Find if a slot near this touch is already active, or allocate a free slot without wiping other fingers!
                int assignedPid = -1;
                for (int i = 0; i < 4; ++i) {
                    if (s_pointers[i].active) {
                        int dx = ev.x - s_pointers[i].curX;
                        int dy = ev.y - s_pointers[i].curY;
                        if (dx * dx + dy * dy <= 3600) {
                            assignedPid = i;
                            break;
                        }
                    }
                }
                if (assignedPid < 0) {
                    if (!s_pointers[pid].active) {
                        assignedPid = pid;
                    } else {
                        for (int i = 0; i < 4; ++i) {
                            if (!s_pointers[i].active) {
                                assignedPid = i;
                                break;
                            }
                        }
                    }
                }
                if (assignedPid < 0) assignedPid = pid;
                pid = assignedPid;

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

                // 1. Virtual Joystick (Bottom-Left) — only if not covered by an open window or TOP_MOST popup (such as Emote bar)
                bool bCoveredByPopup = false;
                {
                    UI::CWindow* pRoot = CAndroidWindowHierarchyAccessor::EnsureRootWindow();
                    UI::CWindow* pHit = pRoot ? CAndroidWindowHierarchyAccessor::PickWindowAtReadOnly(pRoot, ev.x, ev.y) : nullptr;
                    for (UI::CWindow* pW = pHit; pW != nullptr; pW = pW->GetParent()) {
                        const char* wn = pW->GetName();
                        if (wn && (strcmp(wn, "TOP_MOST") == 0 || strcmp(wn, "UI") == 0)) {
                            bCoveredByPopup = true;
                            break;
                        }
                    }
                }
                if (bInGameWorld && !s_bJoyActive && !bCoveredByPopup && (jdx * jdx + jdy * jdy <= joyCatchRadius * joyCatchRadius)) {
                    slot.role = TOUCH_ROLE_JOYSTICK;
                    s_bJoyActive = true;
                    UpdateJoystickFromPoint(ev.x, ev.y);
                    UI::CWindowManager::Instance().RunMouseMove(-5000, -5000);
                    PyRun_SimpleString(
                        "try:\n"
                        "    import constInfo\n"
                        "    if hasattr(constInfo, 'CheckAutoCloseMobileChat'): constInfo.CheckAutoCloseMobileChat(-1, -1)\n"
                        "    if hasattr(constInfo, 'DismissMobileTooltipsOnTap'): constInfo.DismissMobileTooltipsOnTap(-5000, -5000)\n"
                        "except:\n"
                        "    pass\n"
                    );
                }
                // 2. Interactive UI Window / HUD Button (with Smart Touch Magnetism for buttons & scrollbars)
                else if ([&]() {
                    int mx = ev.x, my = ev.y;
                    MapTouchThroughScaledWindows(ev.x, ev.y, &mx, &my);
                    bool snappedBtn = false, isThumb = false;
                    ApplySmartTouchMagnetism(mx, my, &mx, &my, &snappedBtn, &isThumb);
                    s_iLastTouchDownX = mx;
                    s_iLastTouchDownY = my;
                    s_bLastTouchSnappedToButton = snappedBtn;
                    return IsPointOnUILayer(mx, my);
                }()) {
                    int mx = s_iLastTouchDownX, my = s_iLastTouchDownY;
                    slot.role = TOUCH_ROLE_UI;
                    slot.isDraggingWindow = false;
                    slot.holdDragCancelled = false;
                    slot.canTouchScroll = false;
                    slot.isTouchScrolling = false;
                    slot.pHoldDragWnd = nullptr;
                    UpdateUiTouchDownFlag();
                    g_AndroidMouseX = mx;
                    g_AndroidMouseY = my;

                    // Check if the touched UI widget belongs to the Chat system; if not, force-close open ChatWindow!
                    UI::CWindow* pTouched = UI::CWindowManager::Instance().GetPointWindow();
                    bool isChatWidget = false;
                    bool isScrollDragThumb = false;
                    for (UI::CWindow* pWalk = pTouched; pWalk != nullptr; pWalk = pWalk->GetParent()) {
                        if (dynamic_cast<UI::CDragButton*>(pWalk) != nullptr) {
                            isScrollDragThumb = true;
                        }
                        const char* wName = pWalk->GetName();
                        if (wName && (strcmp(wName, "ChatWindow") == 0 ||
                                      strcmp(wName, "ChatInputSet") == 0 ||
                                      strcmp(wName, "Chat Line") == 0 ||
                                      strcmp(wName, "ChatModeButton") == 0 ||
                                      strcmp(wName, "ChatWindow:ChatButton") == 0 ||
                                      strcmp(wName, "ChatTabButton") == 0 ||
                                      strcmp(wName, "ChatResizeCorner") == 0 ||
                                      strcmp(wName, "MobileChatCatcher") == 0)) {
                            isChatWidget = true;
                            break;
                        }
                    }

                    char tapCheckCmd[320];
                    snprintf(tapCheckCmd, sizeof(tapCheckCmd),
                        "try:\n"
                        "    import constInfo\n"
                        "    if hasattr(constInfo, 'CheckAutoCloseMobileChat'): constInfo.CheckAutoCloseMobileChat(%d, %d)\n"
                        "    if hasattr(constInfo, 'DismissMobileTooltipsOnTap'): constInfo.DismissMobileTooltipsOnTap(%d, %d)\n"
                        "except:\n"
                        "    pass\n", isChatWidget ? mx : -1, isChatWidget ? my : -1, mx, my);
                    PyRun_SimpleString(tapCheckCmd);

                    // 1. Run LeftButtonDown FIRST so Python's SelectItemSlot sets _mobilePendingHoldSlot on occupied item slots!
                    UI::CWindowManager::Instance().RunMouseMove(mx, my);
                    UI::CWindowManager::Instance().RunMouseLeftButtonDown(mx, my);

                    // 2. Now check if this touch is inside a scrollable window body OR on a draggable empty slot / window edge
                    if (!s_bLastTouchSnappedToButton && !isScrollDragThumb && !IsMobileItemHoldOrAttachedActive() && CheckMobileTouchScrollableAt(mx, my)) {
                        slot.canTouchScroll = true;
                        slot.pHoldDragWnd = nullptr;
                    } else if (!s_bLastTouchSnappedToButton && !IsMobileItemHoldOrAttachedActive()) {
                        slot.pHoldDragWnd = FindHoldDraggableWindow(mx, my);
                    }
                }
                // 3. 3D World (Orbit / Pinch-to-Zoom / Fast-Tap)
                else {
                    s_iLastTouchDownX = ev.x;
                    s_iLastTouchDownY = ev.y;
                    g_AndroidMouseX = ev.x;
                    g_AndroidMouseY = ev.y;
                    s_bLastTouchSnappedToButton = false;
                    slot.role = TOUCH_ROLE_WORLD;
                    s_fCamVelRoll *= 0.25f;
                    s_fCamVelPitch *= 0.25f;
                    PyRun_SimpleString(
                   "try:\n"
                   "    import constInfo\n"
                   "    if hasattr(constInfo, 'CheckAutoCloseMobileChat'): constInfo.CheckAutoCloseMobileChat(-1, -1)\n"
                   "    if hasattr(constInfo, 'DismissMobileTooltipsOnTap'): constInfo.DismissMobileTooltipsOnTap(-5000, -5000)\n"
                   "except:\n"
                   "    pass\n"
               );
               UI::CWindowManager::Instance().RunMouseMove(ev.x, ev.y);
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
                    DWORD heldMs = ELTimer_GetMSec() - slot.downTimeMs;
                    int totalDx = ev.x - slot.startX;
                    int totalDy = ev.y - slot.startY;
                    int distSq = totalDx * totalDx + totalDy * totalDy;

                    // If holding/detaching an occupied item slot, NEVER hijack into window drag or scroll!
                    if (IsMobileItemHoldOrAttachedActive()) {
                        slot.holdDragCancelled = true;
                        slot.pHoldDragWnd = nullptr;
                        slot.canTouchScroll = false;
                        slot.isTouchScrolling = false;
                    }

                    // Universal Touch-Scrolling inside any window with a visible ScrollBar
                    if (slot.canTouchScroll && !slot.isDraggingWindow) {
                        if (!slot.isTouchScrolling && abs(totalDy) > 7 && abs(totalDy) >= abs(totalDx)) {
                            slot.isTouchScrolling = true;
                            slot.holdDragCancelled = true;
                            slot.pHoldDragWnd = nullptr;
                            UI::CWindowManager::Instance().RunMouseLeftButtonUp(-5000, -5000);
                        }
                        if (slot.isTouchScrolling) {
                            UpdateMobileTouchScroll(stepDy);
                            continue;
                        }
                    }

                    if (!slot.isDraggingWindow && !slot.holdDragCancelled && !slot.isTouchScrolling) {
                        if (heldMs >= 120 && slot.pHoldDragWnd != nullptr && distSq > 16) {
                            slot.isDraggingWindow = true;
                            UI::CWindowManager::Instance().RunMouseLeftButtonUp(-5000, -5000);
                            PyRun_SimpleString(
                             "try:\n"
                             "    import mouseModule, constInfo\n"
                             "    if mouseModule.mouseController.isAttached():\n"
                             "        mouseModule.mouseController.DeattachObject()\n"
                             "    if hasattr(constInfo, 'DismissMobileTooltipsOnTap'):\n"
                             "        constInfo.DismissMobileTooltipsOnTap(-5000, -5000)\n"
                             "except:\n"
                             "    pass\n"
                         );
                        }
                    }

                    if (slot.isDraggingWindow && slot.pHoldDragWnd != nullptr) {
                        long wx = 0, wy = 0;
                        slot.pHoldDragWnd->GetPosition(&wx, &wy);
                        long newX = wx + stepDx;
                        long newY = wy + stepDy;
                        if (newX < -slot.pHoldDragWnd->GetWidth() + 48) newX = -slot.pHoldDragWnd->GetWidth() + 48;
                        if (newX > g_AndroidScreenWidth - 48) newX = g_AndroidScreenWidth - 48;
                        if (newY < 0) newY = 0;
                        if (newY > g_AndroidScreenHeight - 24) newY = g_AndroidScreenHeight - 24;
                        slot.pHoldDragWnd->SetPosition(newX, newY);
                        slot.pHoldDragWnd->OnMoveWindow(newX, newY);
                    } else {
                        int mx = ev.x, my = ev.y;
                        if (s_bLastTouchSnappedToButton && distSq <= 576) {
                            mx = s_iLastTouchDownX;
                            my = s_iLastTouchDownY;
                        } else {
                            MapTouchThroughScaledWindows(ev.x, ev.y, &mx, &my);
                        }
                        g_AndroidMouseX = mx;
                        g_AndroidMouseY = my;
                        UI::CWindowManager::Instance().RunMouseMove(mx, my);
                        SanitizeButtonChildPickFlags(UI::CWindowManager::Instance().GetPointWindow());
                    }
                }
                else if (slot.role == TOUCH_ROLE_WORLD) {
                    g_AndroidMouseX = ev.x;
                    g_AndroidMouseY = ev.y;
                    UI::CWindowManager::Instance().RunMouseMove(ev.x, ev.y);
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
                        if (!slot.isDraggingCamera && (totalDx * totalDx + totalDy * totalDy > 196)) {
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
            else if (maskedAction == 6 || maskedAction == 1) { // ACTION_POINTER_UP or ACTION_UP
                int targetSlot = FindBestActiveSlotForPoint(pid, ev.x, ev.y, false);
                if (targetSlot >= 0) {
                    ReleasePointerSlot(targetSlot, ev.x, ev.y, maskedAction);
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

        // Double-Tap Enemy Auto-Aim & Attack (cancelled immediately if user steers with joystick)
        if (s_bJoyActive && s_dwMobileAutoAttackVID != 0) {
            s_dwMobileAutoAttackVID = 0;
            s_bAutoAttackMoveActive = false;
            if (CPythonPlayer::InstancePtr())
                CPythonPlayer::Instance().SetAttackKeyState(false);
        } else if (s_dwMobileAutoAttackVID != 0 && CPythonPlayer::InstancePtr() && CPythonCharacterManager::InstancePtr()) {
            CAndroidPlayerAccessor& rkPlayer = static_cast<CAndroidPlayerAccessor&>(CPythonPlayer::Instance());
            CInstanceBase* pMain = CPythonCharacterManager::Instance().GetMainInstancePtr();
            CInstanceBase* pTarget = CPythonCharacterManager::Instance().GetInstancePtr(s_dwMobileAutoAttackVID);
            if (!pMain || !pTarget || pTarget->IsDead() || rkPlayer.GetTargetVID() != s_dwMobileAutoAttackVID) {
                s_dwMobileAutoAttackVID = 0;
                s_bAutoAttackMoveActive = false;
                rkPlayer.SetAttackKeyState(false);
            } else if (!pMain->IsUsingSkill() && pCam) {
                TPixelPosition kMyPos, kTargetPos;
                pMain->NEW_GetPixelPosition(&kMyPos);
                pTarget->NEW_GetPixelPosition(&kTargetPos);
                // In Metin2 3D world space, wx = +kPos.x and wy = -kPos.y
                float wdx = kTargetPos.x - kMyPos.x;
                float wdy = (-kTargetPos.y) - (-kMyPos.y);
                float dist = hypotf(wdx, wdy);
                if (dist > 1.0f) {
                    const D3DXMATRIX& matV = pCam->GetViewMatrix();
                    // Camera horizontal Right unit vector in world XY is (matV._11, matV._21)
                    // Camera horizontal Forward unit vector in world XY is (-matV._21, matV._11)
                    float rx = matV._11, ry = matV._21;
                    float rLen = hypotf(rx, ry);
                    if (rLen > 0.001f) { rx /= rLen; ry /= rLen; } else { rx = 1.0f; ry = 0.0f; }
                    float fx = -ry, fy = rx;

                    // Convert world vector to camera-relative joystick axes (joyX = right, joyY = down/towards camera)
                    float joyX = (wdx * rx + wdy * ry) / dist;
                    float joyY = -(wdx * fx + wdy * fy) / dist;
                    float dot = -joyY;
                    if (dot < -1.0f) dot = -1.0f;
                    else if (dot > 1.0f) dot = 1.0f;
                    float ret = acosf(dot) * (180.0f / 3.14159265f);
                    if (joyX < 0.0f) ret = 360.0f - ret;
                    float fDeg = 360.0f - ret;

                    s_fAutoAttackMoveDeg = fDeg;
                    if (dist > 185.0f) {
                        s_bAutoAttackMoveActive = true;
                        rkPlayer.SetAttackKeyState(false);
                        rkPlayer.NEW_MoveToDirection(fDeg);
                    } else {
                        s_bAutoAttackMoveActive = false;
                        rkPlayer.SetAttackKeyState(true);
                    }
                }
            }
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
		if (PollAndroidLiveEditText(env)) {
            pthread_mutex_lock(&s_inputQueueMutex);
            s_keyTail = s_keyHead;
            pthread_mutex_unlock(&s_inputQueueMutex);
        }

        while (true) {
            pthread_mutex_lock(&s_inputQueueMutex);
            if (s_keyTail == s_keyHead) {
                pthread_mutex_unlock(&s_inputQueueMutex);
                break;
            }
            AndroidKeyEvt kev = s_keyQueue[s_keyTail];
            s_keyTail = (s_keyTail + 1) & 63;
            pthread_mutex_unlock(&s_inputQueueMutex);
            // NOTE: In Android KeyEvent, KEYCODE_DEL is 67 (8 is KEYCODE_1!). Only check keyCode == 67 or unicodeChar 8/127.
            if (kev.keyCode == 67 || kev.unicodeChar == 8 || kev.unicodeChar == 127) {
                PyRun_SimpleString("import ime\nime.PasteBackspace()\n");
                SyncMobileLiveTypingToPython();
            } else if (kev.keyCode == 66 || kev.keyCode == 160 || kev.unicodeChar == 10 || kev.unicodeChar == 13) {
                UI::CWindow* pBefore = UI::CWindowManager::Instance().GetActivateWindow();
                UI::CWindowManager::Instance().RunIMEReturnEvent();
                UI::CWindow* pAfter = UI::CWindowManager::Instance().GetActivateWindow();
                if (!pAfter || pAfter == pBefore) {
                    AndroidToggleSoftKeyboard(false);
                }
            } else if (kev.keyCode == 61) {
                UI::CWindowManager::Instance().RunIMETabEvent();
            } else if (kev.unicodeChar >= 32 || (kev.keyCode >= 7 && kev.keyCode <= 16)) {
                uint16_t ch = (kev.unicodeChar >= 32) ? (uint16_t)kev.unicodeChar : (uint16_t)('0' + (kev.keyCode - 7));
                PasteUnicodeCharToMetin2IME(ch);
                SyncMobileLiveTypingToPython();
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
#ifdef ENABLE_OPENGL
     ++s_uMobileRenderFrame;
     if (s_bUiScaleActive && CStateManager::InstancePtr()) {
         CGraphicTextInstance::FlushBatchRender();
         CStateManager::Instance().SetTransform(D3DTS_VIEW, &s_savedUiViewMat);
         s_bUiScaleActive = false;
     }
     if (s_bPendingGroundLeftUp) {
         s_bPendingGroundLeftUp = false;
         UI::CWindowManager::Instance().RunMouseLeftButtonUp(s_iPendingGroundUpX, s_iPendingGroundUpY);
         if (!s_bUiTouchDown) {
             UI::CWindowManager::Instance().RunMouseMove(-5000, -5000);
         }
     }
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

JNIEXPORT void JNICALL Java_com_worldonline_client_NativeLib_touchEvent(JNIEnv* env, jobject obj, jint action, jfloat x, jfloat y) {
    int pid = (action >> 8) & 0x3;
    int masked = (int)(action & 0xFF);
    if (masked == 0 || masked == 5) {
        s_dwLastTouchDownMs = ELTimer_GetMSec();
        EnsureAndroidLiveEditTextOnUiThread(env);
    }
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

JNIEXPORT void JNICALL Java_com_worldonline_client_NativeLib_multiTouchEvent(JNIEnv* env, jobject obj, jint pointerId, jint action, jfloat x, jfloat y) {
    int pid = (int)(pointerId & 3);
    int masked = (int)(action & 0xFF);
    if (masked == 0 || masked == 5) {
        s_dwLastTouchDownMs = ELTimer_GetMSec();
        EnsureAndroidLiveEditTextOnUiThread(env);
    }
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

JNIEXPORT void JNICALL Java_com_worldonline_client_NativeLib_keyEvent(JNIEnv* env, jobject obj, jint keyCode, jint unicodeChar) {
#ifdef __APPLE__
    if (keyCode == 67 || unicodeChar == 8 || unicodeChar == 127) {
        __atomic_fetch_add(&s_uIosBackspaceEventCount, 1, __ATOMIC_RELAXED);
    }
#endif
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

namespace {
    constexpr int kMixSampleRate = 44100;
    constexpr int kMixSliceFrames = 512; // ~11.6ms low-latency mixing buffer
    constexpr int kMaxMixVoices = 64;    // 64 simultaneous voices (never cuts off active sounds)
    constexpr int kMaxSameSoundVoices = 6;

    struct SDecodedSfxBuffer {
        std::vector<int16_t> pcm; // 44100 Hz 16-bit mono PCM
        bool valid = false;
        bool isCompressedFallback = false;
        std::string diskPath;
    };

    struct SMixVoice {
        int soundId = -1;
        const std::vector<int16_t>* pcm = nullptr;
        uint32_t posFp = 0;   // 24.8 fixed-point sample cursor
        uint32_t stepFp = 256; // 256 = 1.0x pitch rate
        int32_t gainL = 0;    // 8-bit fixed-point gain (256 = 1.0)
        int32_t gainR = 0;
        DWORD startMs = 0;
        bool active = false;
    };

    struct SSfxPlayReq {
        int soundId = 0;
        float volume = 1.0f;
        float pan = 0.0f;
        float leftVol = 1.0f;
        float rightVol = 1.0f;
        std::string lowerPath;
        std::string diskPath;
        std::vector<uint8_t> rawPackBytes;
    };

    static pthread_mutex_t s_sfxQueueMutex = PTHREAD_MUTEX_INITIALIZER;
    static std::deque<SSfxPlayReq> s_sfxQueue;
    static pthread_mutex_t s_compQueueMutex = PTHREAD_MUTEX_INITIALIZER;
    static std::deque<SSfxPlayReq> s_compQueue;

    static volatile bool s_bSfxThreadsStarted = false;
    static pthread_t s_sfxMixerThread;
    static pthread_t s_sfxCompThread;

// Enabled on BOTH Android and iOS so iOS uses the 64-voice C++ in-memory WAV mixer
    static int16_t DecodeImaAdpcmNib(uint8_t nibble, int32_t& predictor, int32_t& stepIndex) {
        static const int16_t kStepTable[89] = {
            7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
            50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230,
            253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963,
            1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327,
            3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487,
            12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
        };
        static const int8_t kIndexTable[16] = {
            -1, -1, -1, -1, 2, 4, 6, 8,
            -1, -1, -1, -1, 2, 4, 6, 8
        };
        int32_t step = kStepTable[stepIndex];
        int32_t diff = step >> 3;
        if (nibble & 4) diff += step;
        if (nibble & 2) diff += (step >> 1);
        if (nibble & 1) diff += (step >> 2);
        if (nibble & 8) predictor -= diff;
        else predictor += diff;
        if (predictor > 32767) predictor = 32767;
        else if (predictor < -32768) predictor = -32768;
        stepIndex += kIndexTable[nibble & 0x0F];
        if (stepIndex < 0) stepIndex = 0;
        else if (stepIndex > 88) stepIndex = 88;
        return (int16_t)predictor;
    }

    static int16_t DecodeMsAdpcmNib(uint8_t nibble, int32_t c1, int32_t c2, int16_t& delta, int16_t& samp1, int16_t& samp2) {
        static const int32_t kAdaptTable[16] = {
            230, 230, 230, 230, 307, 409, 512, 614,
            768, 614, 512, 409, 307, 230, 230, 230
        };
        int32_t snib = (nibble >= 8) ? ((int32_t)nibble - 16) : (int32_t)nibble;
        int32_t pred = (((int32_t)samp1 * c1) + ((int32_t)samp2 * c2)) >> 8;
        int32_t cur = pred + snib * (int32_t)delta;
        if (cur > 32767) cur = 32767;
        else if (cur < -32768) cur = -32768;
        samp2 = samp1;
        samp1 = (int16_t)cur;
        int32_t nextDelta = ((int32_t)delta * kAdaptTable[nibble & 0x0F]) >> 8;
        delta = (int16_t)((nextDelta < 16) ? 16 : nextDelta);
        return (int16_t)cur;
    }

    static bool DecodeWavToMono44100(const uint8_t* data, size_t size, std::vector<int16_t>& outSamples) {
        if (!data || size < 44) return false;
        if (memcmp(data, "RIFF", 4) != 0 || memcmp(data + 8, "WAVE", 4) != 0)
            return false;

        uint16_t fmtTag = 0, channels = 0, blockAlign = 0, bitsPerSample = 0;
        uint32_t sampleRate = 0;
        const uint8_t* fmtPtr = nullptr;
        uint32_t fmtSize = 0;
        const uint8_t* dataPtr = nullptr;
        uint32_t dataSize = 0;

        size_t pos = 12;
        while (pos + 8 <= size) {
            uint32_t chunkId = 0, chunkSz = 0;
            memcpy(&chunkId, data + pos, 4);
            memcpy(&chunkSz, data + pos + 4, 4);
            pos += 8;
            if (pos + chunkSz > size) chunkSz = (uint32_t)(size - pos);

            if (chunkId == 0x20746D66u && chunkSz >= 16) { // 'fmt '
                fmtPtr = data + pos;
                fmtSize = chunkSz;
                memcpy(&fmtTag, fmtPtr + 0, 2);
                memcpy(&channels, fmtPtr + 2, 2);
                memcpy(&sampleRate, fmtPtr + 4, 4);
                memcpy(&blockAlign, fmtPtr + 12, 2);
                memcpy(&bitsPerSample, fmtPtr + 14, 2);
            } else if (chunkId == 0x61746164u) { // 'data'
                dataPtr = data + pos;
                dataSize = chunkSz;
                break;
            }
            pos += chunkSz + (chunkSz & 1);
        }

        if (!dataPtr || dataSize == 0 || channels == 0 || channels > 2 || sampleRate < 4000 || sampleRate > 96000)
            return false;

        std::vector<int16_t> monoSrc;
        if (fmtTag == 1) { // Standard PCM (8-bit or 16-bit)
            if (bitsPerSample == 16) {
                size_t frames = dataSize / (channels * 2);
                monoSrc.resize(frames);
                const int16_t* src = (const int16_t*)dataPtr;
                if (channels == 1) {
                    memcpy(monoSrc.data(), src, frames * sizeof(int16_t));
                } else {
                    for (size_t i = 0; i < frames; ++i)
                        monoSrc[i] = (int16_t)(((int32_t)src[i * 2] + (int32_t)src[i * 2 + 1]) >> 1);
                }
            } else if (bitsPerSample == 8) {
                size_t frames = dataSize / channels;
                monoSrc.resize(frames);
                if (channels == 1) {
                    for (size_t i = 0; i < frames; ++i)
                        monoSrc[i] = (int16_t)(((int32_t)dataPtr[i] - 128) << 8);
                } else {
                    for (size_t i = 0; i < frames; ++i) {
                        int32_t l = ((int32_t)dataPtr[i * 2] - 128) << 8;
                        int32_t r = ((int32_t)dataPtr[i * 2 + 1] - 128) << 8;
                        monoSrc[i] = (int16_t)((l + r) >> 1);
                    }
                }
            } else {
                return false;
            }
        } else if (fmtTag == 2 && blockAlign >= 7 * channels) { // Microsoft ADPCM (4-bit)
            int32_t coeff1[7] = { 256, 512, 0, 192, 240, 460, 392 };
            int32_t coeff2[7] = { 0, -256, 0, 64, 0, -208, -232 };
            if (fmtPtr && fmtSize >= 22) {
                uint16_t numCoef = 0;
                memcpy(&numCoef, fmtPtr + 20, 2);
                for (uint16_t c = 0; c < numCoef && c < 7 && (22u + c * 4u + 4u <= fmtSize); ++c) {
                    int16_t a1 = 0, a2 = 0;
                    memcpy(&a1, fmtPtr + 22 + c * 4, 2);
                    memcpy(&a2, fmtPtr + 24 + c * 4, 2);
                    coeff1[c] = a1;
                    coeff2[c] = a2;
                }
            }
            size_t numBlocks = (dataSize + blockAlign - 1) / blockAlign;
            monoSrc.reserve(numBlocks * blockAlign * 2 / channels);
            for (size_t b = 0; b < numBlocks; ++b) {
                size_t blkOff = b * blockAlign;
                if (blkOff + 7 * channels > dataSize) break;
                size_t curBlkSize = std::min<size_t>(blockAlign, dataSize - blkOff);
                const uint8_t* blk = dataPtr + blkOff;

                if (channels == 1) {
                    uint8_t pIdx = blk[0] < 7 ? blk[0] : 0;
                    int16_t delta = 0, samp1 = 0, samp2 = 0;
                    memcpy(&delta, blk + 1, 2);
                    memcpy(&samp1, blk + 3, 2);
                    memcpy(&samp2, blk + 5, 2);
                    monoSrc.push_back(samp2);
                    monoSrc.push_back(samp1);
                    int32_t c1 = coeff1[pIdx], c2 = coeff2[pIdx];
                    for (size_t i = 7; i < curBlkSize; ++i) {
                        uint8_t byte = blk[i];
                        monoSrc.push_back(DecodeMsAdpcmNib(byte >> 4,   c1, c2, delta, samp1, samp2));
                        monoSrc.push_back(DecodeMsAdpcmNib(byte & 0x0F, c1, c2, delta, samp1, samp2));
                    }
                } else {
                    uint8_t pIdxL = blk[0] < 7 ? blk[0] : 0;
                    uint8_t pIdxR = blk[1] < 7 ? blk[1] : 0;
                    int16_t deltaL = 0, deltaR = 0, samp1L = 0, samp1R = 0, samp2L = 0, samp2R = 0;
                    memcpy(&deltaL, blk + 2, 2); memcpy(&deltaR, blk + 4, 2);
                    memcpy(&samp1L, blk + 6, 2); memcpy(&samp1R, blk + 8, 2);
                    memcpy(&samp2L, blk + 10, 2); memcpy(&samp2R, blk + 12, 2);
                    monoSrc.push_back((int16_t)(((int32_t)samp2L + (int32_t)samp2R) >> 1));
                    monoSrc.push_back((int16_t)(((int32_t)samp1L + (int32_t)samp1R) >> 1));
                    int32_t c1L = coeff1[pIdxL], c2L = coeff2[pIdxL];
                    int32_t c1R = coeff1[pIdxR], c2R = coeff2[pIdxR];
                    for (size_t i = 14; i < curBlkSize; ++i) {
                        uint8_t byte = blk[i];
                        int16_t sL = DecodeMsAdpcmNib(byte >> 4,   c1L, c2L, deltaL, samp1L, samp2L);
                        int16_t sR = DecodeMsAdpcmNib(byte & 0x0F, c1R, c2R, deltaR, samp1R, samp2R);
                        monoSrc.push_back((int16_t)(((int32_t)sL + (int32_t)sR) >> 1));
                    }
                }
            }
        } else if (fmtTag == 0x0011 && blockAlign >= 4 * channels) { // IMA ADPCM (4-bit)
            size_t numBlocks = (dataSize + blockAlign - 1) / blockAlign;
            monoSrc.reserve(numBlocks * (blockAlign - 4 * channels) * 2 / channels);
            for (size_t b = 0; b < numBlocks; ++b) {
                size_t blkOff = b * blockAlign;
                if (blkOff + 4 * channels > dataSize) break;
                size_t curBlkSize = std::min<size_t>(blockAlign, dataSize - blkOff);
                const uint8_t* blk = dataPtr + blkOff;
                int32_t pred[2] = {0, 0};
                int32_t stepIdx[2] = {0, 0};
                for (uint16_t ch = 0; ch < channels; ++ch) {
                    int16_t p0 = 0;
                    memcpy(&p0, blk + ch * 4, 2);
                    pred[ch] = p0;
                    stepIdx[ch] = blk[ch * 4 + 2];
                    if (stepIdx[ch] < 0) stepIdx[ch] = 0;
                    else if (stepIdx[ch] > 88) stepIdx[ch] = 88;
                }
                monoSrc.push_back(channels == 1 ? (int16_t)pred[0] : (int16_t)((pred[0] + pred[1]) >> 1));
                size_t offset = 4 * channels;
                if (channels == 1) {
                    for (; offset < curBlkSize; ++offset) {
                        uint8_t byte = blk[offset];
                        monoSrc.push_back(DecodeImaAdpcmNib(byte & 0x0F, pred[0], stepIdx[0]));
                        monoSrc.push_back(DecodeImaAdpcmNib(byte >> 4,   pred[0], stepIdx[0]));
                    }
                } else {
                    while (offset + 8 <= curBlkSize) {
                        int16_t ch0[8], ch1[8];
                        for (int k = 0; k < 4; ++k) {
                            uint8_t b0 = blk[offset + k];
                            ch0[k * 2 + 0] = DecodeImaAdpcmNib(b0 & 0x0F, pred[0], stepIdx[0]);
                            ch0[k * 2 + 1] = DecodeImaAdpcmNib(b0 >> 4,   pred[0], stepIdx[0]);
                            uint8_t b1 = blk[offset + 4 + k];
                            ch1[k * 2 + 0] = DecodeImaAdpcmNib(b1 & 0x0F, pred[1], stepIdx[1]);
                            ch1[k * 2 + 1] = DecodeImaAdpcmNib(b1 >> 4,   pred[1], stepIdx[1]);
                        }
                        for (int k = 0; k < 8; ++k)
                            monoSrc.push_back((int16_t)(((int32_t)ch0[k] + (int32_t)ch1[k]) >> 1));
                        offset += 8;
                    }
                }
            }
        } else {
            return false;
        }

        if (monoSrc.empty()) return false;
        if (sampleRate == (uint32_t)kMixSampleRate) {
            outSamples = std::move(monoSrc);
        } else {
            uint64_t stepFp = ((uint64_t)sampleRate << 16) / (uint64_t)kMixSampleRate;
            size_t outLen = (size_t)(((uint64_t)monoSrc.size() << 16) / stepFp);
            if (outLen == 0) return false;
            outSamples.resize(outLen);
            size_t maxIdx = monoSrc.size() - 1;
            uint64_t curFp = 0;
            for (size_t i = 0; i < outLen; ++i) {
                size_t idx = (size_t)(curFp >> 16);
                int32_t frac = (int32_t)(curFp & 0xFFFFu);
                int32_t s0 = monoSrc[idx];
                int32_t s1 = monoSrc[(idx < maxIdx) ? (idx + 1) : maxIdx];
                outSamples[i] = (int16_t)(s0 + (((s1 - s0) * frac) >> 16));
                curFp += stepFp;
            }
        }
        return true;
    }

    #if !defined(__APPLE__)
    // Separate helper thread ONLY for rare compressed (.mp3 / .asi) sound effects so SoundPool.load never touches the 64-voice PCM mixer thread
    static void* AndroidCompressedSfxThreadProc(void*) {
        if (!g_pJavaVM) return nullptr;
        JNIEnv* env = nullptr;
        if (g_pJavaVM->AttachCurrentThread(&env, nullptr) != JNI_OK || !env) return nullptr;

        jclass spCls = env->FindClass("android/media/SoundPool");
        jobject spObj = nullptr;
        jmethodID loadMid = nullptr, playMid = nullptr;
        if (spCls) {
            jmethodID spCtor = env->GetMethodID(spCls, "<init>", "(III)V");
            spObj = env->NewObject(spCls, spCtor, 8, 3, 0);
            env->ExceptionClear();
            loadMid = env->GetMethodID(spCls, "load", "(Ljava/lang/String;I)I");
            playMid = env->GetMethodID(spCls, "play", "(IFFIIF)I");
        }

        struct SCompEntry { int soundId; DWORD loadMs; DWORD lastPlayMs; };
        std::unordered_map<int, SCompEntry> compMap;
        struct SPendingComp { int soundId; float lVol; float rVol; DWORD readyMs; };
        std::vector<SPendingComp> pending;
        std::deque<SSfxPlayReq> localReqs;

        while (true) {
            pthread_mutex_lock(&s_compQueueMutex);
            if (!s_compQueue.empty()) localReqs.swap(s_compQueue);
            pthread_mutex_unlock(&s_compQueueMutex);

            DWORD nowMs = ELTimer_GetMSec();
            for (const SSfxPlayReq& req : localReqs) {
                if (!req.rawPackBytes.empty() && !req.diskPath.empty()) {
                    if (FILE* fp = fopen(req.diskPath.c_str(), "wb")) {
                        fwrite(req.rawPackBytes.data(), 1, req.rawPackBytes.size(), fp);
                        fclose(fp);
                    }
                }
                if (!spObj || !loadMid || !playMid) continue;
                auto it = compMap.find(req.soundId);
                if (it == compMap.end()) {
                    jstring jpath = env->NewStringUTF(req.diskPath.c_str());
                    int id = (int)env->CallIntMethod(spObj, loadMid, jpath, 1);
                    env->ExceptionClear();
                    env->DeleteLocalRef(jpath);
                    compMap[req.soundId] = { id, nowMs, nowMs };
                    if (id > 0 && pending.size() < 24)
                        pending.push_back({ id, req.leftVol, req.rightVol, nowMs + 55 });
                } else if (it->second.soundId > 0) {
                    if (nowMs - it->second.lastPlayMs < 12) continue;
                    it->second.lastPlayMs = nowMs;
                    if (nowMs - it->second.loadMs < 55) {
                        if (pending.size() < 24)
                            pending.push_back({ it->second.soundId, req.leftVol, req.rightVol, it->second.loadMs + 55 });
                    } else {
                        env->CallIntMethod(spObj, playMid, (jint)it->second.soundId, (jfloat)req.leftVol, (jfloat)req.rightVol, 1, 0, 1.0f);
                        env->ExceptionClear();
                    }
                }
            }
            localReqs.clear();

            for (size_t i = 0; i < pending.size(); ) {
                if (nowMs >= pending[i].readyMs) {
                    env->CallIntMethod(spObj, playMid, (jint)pending[i].soundId, (jfloat)pending[i].lVol, (jfloat)pending[i].rVol, 1, 0, 1.0f);
                    env->ExceptionClear();
                    pending.erase(pending.begin() + i);
                } else {
                    ++i;
                }
            }
            usleep(6000);
        }
        return nullptr;
    }
#endif

#ifdef __APPLE__
    struct SfxIOSAudioStreamBasicDesc {
        double mSampleRate;
        uint32_t mFormatID;
        uint32_t mFormatFlags;
        uint32_t mBytesPerPacket;
        uint32_t mFramesPerPacket;
        uint32_t mBytesPerFrame;
        uint32_t mChannelsPerFrame;
        uint32_t mBitsPerChannel;
        uint32_t mReserved;
    };
    struct SfxIOSAudioQueueBuffer {
        uint32_t mAudioDataBytesCapacity;
        void* mAudioData;
        uint32_t mAudioDataByteSize;
        void* mUserData;
        uint32_t mPacketDescriptionCapacity;
        void* mPacketDescriptions;
        uint32_t mPacketDescriptionCount;
    };
    typedef void* SfxIOSAudioQueueRef;
    typedef SfxIOSAudioQueueBuffer* SfxIOSAudioQueueBufferRef;
    typedef void (*SfxIOSAudioQueueOutputCB)(void*, SfxIOSAudioQueueRef, SfxIOSAudioQueueBufferRef);
    typedef int32_t (*PFN_SfxAQNewOutput)(const SfxIOSAudioStreamBasicDesc*, SfxIOSAudioQueueOutputCB, void*, void*, void*, uint32_t, SfxIOSAudioQueueRef*);
    typedef int32_t (*PFN_SfxAQAllocBuf)(SfxIOSAudioQueueRef, uint32_t, SfxIOSAudioQueueBufferRef*);
    typedef int32_t (*PFN_SfxAQEnqueueBuf)(SfxIOSAudioQueueRef, SfxIOSAudioQueueBufferRef, uint32_t, const void*);
    typedef int32_t (*PFN_SfxAQStart)(SfxIOSAudioQueueRef, const void*);

    static pthread_mutex_t s_iosVoicesMutex = PTHREAD_MUTEX_INITIALIZER;
    static SMixVoice s_iosVoices[kMaxMixVoices];
    static PFN_SfxAQEnqueueBuf s_pfnSfxAQEnqueue = nullptr;

    static void SfxIOSAudioOutputCallback(void*, SfxIOSAudioQueueRef outAQ, SfxIOSAudioQueueBufferRef outBuf) {
        if (!outBuf || !outBuf->mAudioData) return;
        constexpr int kIosBufFrames = 1024;
        int32_t mixAccum[kIosBufFrames * 2];
        memset(mixAccum, 0, sizeof(mixAccum));
        int16_t* stereoOut = (int16_t*)outBuf->mAudioData;
        bool anyActive = false;

        pthread_mutex_lock(&s_iosVoicesMutex);
        for (int v = 0; v < kMaxMixVoices; ++v) {
            SMixVoice& vc = s_iosVoices[v];
            if (!vc.active || !vc.pcm || vc.pcm->empty()) continue;
            anyActive = true;

            const int16_t* src = vc.pcm->data();
            const uint32_t totalSamples = (uint32_t)vc.pcm->size();
            const uint32_t endFp = totalSamples << 8;
            uint32_t curFp = vc.posFp;
            const uint32_t stepFp = vc.stepFp;
            const int32_t gL = vc.gainL;
            const int32_t gR = vc.gainR;

            for (int f = 0; f < kIosBufFrames; ++f) {
                if (curFp >= endFp) {
                    vc.active = false;
                    break;
                }
                int32_t s = src[curFp >> 8];
                mixAccum[f * 2 + 0] += (s * gL) >> 8;
                mixAccum[f * 2 + 1] += (s * gR) >> 8;
                curFp += stepFp;
            }
            vc.posFp = curFp;
            if (curFp >= endFp) vc.active = false;
        }
        pthread_mutex_unlock(&s_iosVoicesMutex);

        if (anyActive) {
            for (int i = 0; i < kIosBufFrames * 2; ++i) {
                int32_t x = mixAccum[i];
                if (x > 24576) {
                    int32_t excess = x - 24576;
                    x = 24576 + (excess * 8191) / (8191 + excess);
                } else if (x < -24576) {
                    int32_t excess = -24576 - x;
                    x = -24576 - (excess * 8191) / (8191 + excess);
                }
                stereoOut[i] = (int16_t)x;
            }
        } else {
            memset(stereoOut, 0, kIosBufFrames * 2 * sizeof(int16_t));
        }
        outBuf->mAudioDataByteSize = kIosBufFrames * 2 * sizeof(int16_t);
        if (s_pfnSfxAQEnqueue) {
            s_pfnSfxAQEnqueue(outAQ, outBuf, 0, nullptr);
        }
    }
#endif

    // Dedicated 64-voice C++ software mixer thread (Android & iOS)
    static void* MobileSfxMixerThreadProc(void*) {
#ifdef __APPLE__
        void* hAQ = dlopen("/System/Library/Frameworks/AudioToolbox.framework/AudioToolbox", RTLD_LAZY);
        if (!hAQ) hAQ = RTLD_DEFAULT;
        PFN_SfxAQNewOutput pfnNewOut = (PFN_SfxAQNewOutput)dlsym(hAQ, "AudioQueueNewOutput");
        PFN_SfxAQAllocBuf  pfnAlloc  = (PFN_SfxAQAllocBuf)dlsym(hAQ, "AudioQueueAllocateBuffer");
        s_pfnSfxAQEnqueue            = (PFN_SfxAQEnqueueBuf)dlsym(hAQ, "AudioQueueEnqueueBuffer");
        PFN_SfxAQStart     pfnStart  = (PFN_SfxAQStart)dlsym(hAQ, "AudioQueueStart");

        SfxIOSAudioQueueRef iosQueue = nullptr;
        if (pfnNewOut && pfnAlloc && s_pfnSfxAQEnqueue && pfnStart) {
            SfxIOSAudioStreamBasicDesc fmt{};
            fmt.mSampleRate = (double)kMixSampleRate;
            fmt.mFormatID = 0x6C70636Du; // 'lpcm'
            fmt.mFormatFlags = 0xCu;     // SignedInteger | Packed
            fmt.mBytesPerPacket = 4;
            fmt.mFramesPerPacket = 1;
            fmt.mBytesPerFrame = 4;
            fmt.mChannelsPerFrame = 2;
            fmt.mBitsPerChannel = 16;
            if (pfnNewOut(&fmt, SfxIOSAudioOutputCallback, nullptr, nullptr, nullptr, 0, &iosQueue) == 0 && iosQueue) {
                for (int i = 0; i < 4; ++i) {
                    SfxIOSAudioQueueBufferRef buf = nullptr;
                    if (pfnAlloc(iosQueue, 1024 * 4, &buf) == 0 && buf) {
                        memset(buf->mAudioData, 0, 1024 * 4);
                        buf->mAudioDataByteSize = 1024 * 4;
                        s_pfnSfxAQEnqueue(iosQueue, buf, 0, nullptr);
                    }
                }
                pfnStart(iosQueue, nullptr);
            }
        }

        std::unordered_map<int, SDecodedSfxBuffer> soundBank;
        std::unordered_map<int, DWORD> compLastPlay;
        std::deque<SSfxPlayReq> localCmds;
        uint32_t jitterSeq = 0;

        while (true) {
            pthread_mutex_lock(&s_sfxQueueMutex);
            if (!s_sfxQueue.empty()) localCmds.swap(s_sfxQueue);
            pthread_mutex_unlock(&s_sfxQueueMutex);

            if (!localCmds.empty()) {
                DWORD nowMs = ELTimer_GetMSec();
                for (SSfxPlayReq& req : localCmds) {
                    auto it = soundBank.find(req.soundId);
                    if (it == soundBank.end()) {
                        SDecodedSfxBuffer dec;
                        dec.diskPath = req.diskPath;
                        if (!req.rawPackBytes.empty()) {
                            if (DecodeWavToMono44100(req.rawPackBytes.data(), req.rawPackBytes.size(), dec.pcm)) {
                                dec.valid = true;
                            } else {
                                dec.isCompressedFallback = true;
                                if (!req.diskPath.empty()) {
                                    if (FILE* fp = fopen(req.diskPath.c_str(), "wb")) {
                                        fwrite(req.rawPackBytes.data(), 1, req.rawPackBytes.size(), fp);
                                        fclose(fp);
                                    }
                                }
                            }
                        } else if (!req.diskPath.empty()) {
                            FILE* fp = fopen(req.diskPath.c_str(), "rb");
                            if (fp) {
                                fseek(fp, 0, SEEK_END);
                                long sz = ftell(fp);
                                fseek(fp, 0, SEEK_SET);
                                if (sz > 44 && sz < (4 * 1024 * 1024)) {
                                    std::vector<uint8_t> buf((size_t)sz);
                                    if (fread(buf.data(), 1, (size_t)sz, fp) == (size_t)sz &&
                                        DecodeWavToMono44100(buf.data(), buf.size(), dec.pcm)) {
                                        dec.valid = true;
                                    } else {
                                        dec.isCompressedFallback = true;
                                    }
                                }
                                fclose(fp);
                            }
                        }
                        it = soundBank.emplace(req.soundId, std::move(dec)).first;
                    }

                    const SDecodedSfxBuffer& sfx = it->second;
                    if (sfx.isCompressedFallback) {
                        if (!sfx.diskPath.empty()) {
                            auto itT = compLastPlay.find(req.soundId);
                            if (itT == compLastPlay.end() || (nowMs - itT->second) >= 25) {
                                compLastPlay[req.soundId] = nowMs;
                                IOSPlayAudioFile3D(sfx.diskPath.c_str(), req.volume, req.pan, 1.0f);
                            }
                        }
                        continue;
                    }
                    if (!sfx.valid || sfx.pcm.empty()) continue;

                    int32_t gL = (int32_t)(req.leftVol * 256.0f);
                    int32_t gR = (int32_t)(req.rightVol * 256.0f);
                    if (gL <= 1 && gR <= 1) continue;

                    pthread_mutex_lock(&s_iosVoicesMutex);
                    int sameCount = 0;
                    int oldestSameIdx = -1;
                    uint32_t oldestSamePos = 0;
                    bool mergedSameFrame = false;

                    for (int v = 0; v < kMaxMixVoices; ++v) {
                        if (s_iosVoices[v].active && s_iosVoices[v].soundId == req.soundId) {
                            if (nowMs - s_iosVoices[v].startMs < 6 && s_iosVoices[v].posFp < (512u << 8)) {
                                s_iosVoices[v].gainL = std::min<int32_t>(320, std::max<int32_t>(s_iosVoices[v].gainL, gL));
                                s_iosVoices[v].gainR = std::min<int32_t>(320, std::max<int32_t>(s_iosVoices[v].gainR, gR));
                                mergedSameFrame = true;
                                break;
                            }
                            ++sameCount;
                            if (s_iosVoices[v].posFp >= oldestSamePos) {
                                oldestSamePos = s_iosVoices[v].posFp;
                                oldestSameIdx = v;
                            }
                        }
                    }
                    if (!mergedSameFrame) {
                        int chosenSlot = -1;
                        if (sameCount >= kMaxSameSoundVoices && oldestSameIdx >= 0) {
                            chosenSlot = oldestSameIdx;
                        } else {
                            for (int v = 0; v < kMaxMixVoices; ++v) {
                                if (!s_iosVoices[v].active) {
                                    chosenSlot = v;
                                    break;
                                }
                            }
                            if (chosenSlot < 0) {
                                int bestIdx = 0;
                                uint32_t maxProgress = 0;
                                for (int v = 0; v < kMaxMixVoices; ++v) {
                                    if (s_iosVoices[v].posFp >= maxProgress) {
                                        maxProgress = s_iosVoices[v].posFp;
                                        bestIdx = v;
                                    }
                                }
                                chosenSlot = bestIdx;
                            }
                        }
                        static const uint32_t kDetuneSteps[4] = { 256, 253, 259, 255 };
                        s_iosVoices[chosenSlot].soundId = req.soundId;
                        s_iosVoices[chosenSlot].pcm = &sfx.pcm;
                        s_iosVoices[chosenSlot].posFp = 0;
                        s_iosVoices[chosenSlot].stepFp = (sameCount > 0) ? kDetuneSteps[(++jitterSeq) & 3] : 256u;
                        s_iosVoices[chosenSlot].gainL = gL;
                        s_iosVoices[chosenSlot].gainR = gR;
                        s_iosVoices[chosenSlot].startMs = nowMs;
                        s_iosVoices[chosenSlot].active = true;
                    }
                    pthread_mutex_unlock(&s_iosVoicesMutex);
                }
                localCmds.clear();
            }
            usleep(4000);
        }
        return nullptr;
#else
        if (!g_pJavaVM) return nullptr;
        JNIEnv* env = nullptr;
        if (g_pJavaVM->AttachCurrentThread(&env, nullptr) != JNI_OK || !env) return nullptr;

        jclass atCls = env->FindClass("android/media/AudioTrack");
        jobject mixTrack = nullptr;
        jmethodID writeMid = nullptr;
        jshortArray mixJArray = env->NewShortArray(kMixSliceFrames * 2);

        jmethodID playMid = nullptr;
        if (atCls) {
            jmethodID minBufMid = env->GetStaticMethodID(atCls, "getMinBufferSize", "(III)I");
            jmethodID ctorMid = env->GetMethodID(atCls, "<init>", "(IIIIII)V");
            playMid = env->GetMethodID(atCls, "play", "()V");
            writeMid = env->GetMethodID(atCls, "write", "([SII)I");

            jint minBuf = env->CallStaticIntMethod(atCls, minBufMid, (jint)kMixSampleRate, (jint)12 /*OUT_STEREO*/, (jint)2 /*PCM_16BIT*/);
            env->ExceptionClear();
            int bufBytes = std::max<int>((int)minBuf, kMixSliceFrames * 4 * 4);
            mixTrack = env->NewObject(atCls, ctorMid, (jint)3 /*STREAM_MUSIC*/, (jint)kMixSampleRate, (jint)12, (jint)2, (jint)bufBytes, (jint)1 /*MODE_STREAM*/);
            env->ExceptionClear();
            if (mixTrack && playMid) {
                env->CallVoidMethod(mixTrack, playMid);
                env->ExceptionClear();
            }
        }

        std::unordered_map<int, SDecodedSfxBuffer> soundBank;
        SMixVoice voices[kMaxMixVoices];
        int16_t stereoOut[kMixSliceFrames * 2];
        int32_t mixAccum[kMixSliceFrames * 2];
        std::deque<SSfxPlayReq> localReqs;
        DWORD lastActiveVoiceMs = ELTimer_GetMSec();
        uint32_t jitterSeq = 0;

        while (true) {
            pthread_mutex_lock(&s_sfxQueueMutex);
            if (!s_sfxQueue.empty()) localReqs.swap(s_sfxQueue);
            pthread_mutex_unlock(&s_sfxQueueMutex);

            DWORD nowMs = ELTimer_GetMSec();

            for (SSfxPlayReq& req : localReqs) {
                auto it = soundBank.find(req.soundId);
                if (it == soundBank.end()) {
                    SDecodedSfxBuffer dec;
                    dec.diskPath = req.diskPath;
                    if (!req.rawPackBytes.empty()) {
                        if (DecodeWavToMono44100(req.rawPackBytes.data(), req.rawPackBytes.size(), dec.pcm)) {
                            dec.valid = true;
                        } else {
                            dec.isCompressedFallback = true;
                        }
                    } else if (!req.diskPath.empty()) {
                        FILE* fp = fopen(req.diskPath.c_str(), "rb");
                        if (fp) {
                            fseek(fp, 0, SEEK_END);
                            long sz = ftell(fp);
                            fseek(fp, 0, SEEK_SET);
                            if (sz > 44 && sz < (4 * 1024 * 1024)) {
                                std::vector<uint8_t> buf((size_t)sz);
                                if (fread(buf.data(), 1, (size_t)sz, fp) == (size_t)sz &&
                                    DecodeWavToMono44100(buf.data(), buf.size(), dec.pcm)) {
                                    dec.valid = true;
                                } else {
                                    dec.isCompressedFallback = true;
                                }
                            }
                            fclose(fp);
                        }
                    }
                    it = soundBank.emplace(req.soundId, std::move(dec)).first;
                }

                const SDecodedSfxBuffer& sfx = it->second;
                if (sfx.isCompressedFallback) {
                    pthread_mutex_lock(&s_compQueueMutex);
                    if (s_compQueue.size() < 32) s_compQueue.push_back(std::move(req));
                    pthread_mutex_unlock(&s_compQueueMutex);
                    continue;
                }
                if (!sfx.valid || sfx.pcm.empty()) continue;

                int32_t gL = (int32_t)(req.leftVol * 256.0f);
                int32_t gR = (int32_t)(req.rightVol * 256.0f);
                if (gL <= 1 && gR <= 1) continue;

                // Check how many instances of this exact soundId are already playing
                int sameCount = 0;
                int oldestSameIdx = -1;
                uint32_t oldestSamePos = 0;
                bool mergedSameFrame = false;

                for (int v = 0; v < kMaxMixVoices; ++v) {
                    if (voices[v].active && voices[v].soundId == req.soundId) {
                        // If triggered within < 6ms (exact same frame), merge volume instead of doubling phase
                        if (nowMs - voices[v].startMs < 6 && voices[v].posFp < (512u << 8)) {
                            voices[v].gainL = std::min<int32_t>(320, std::max<int32_t>(voices[v].gainL, gL));
                            voices[v].gainR = std::min<int32_t>(320, std::max<int32_t>(voices[v].gainR, gR));
                            mergedSameFrame = true;
                            break;
                        }
                        ++sameCount;
                        if (voices[v].posFp >= oldestSamePos) {
                            oldestSamePos = voices[v].posFp;
                            oldestSameIdx = v;
                        }
                    }
                }
                if (mergedSameFrame) continue;

                // Allocate a free voice slot (or reuse oldest instance of the SAME sound if >= 6 are already overlapping)
                int chosenSlot = -1;
                if (sameCount >= kMaxSameSoundVoices && oldestSameIdx >= 0) {
                    chosenSlot = oldestSameIdx;
                } else {
                    for (int v = 0; v < kMaxMixVoices; ++v) {
                        if (!voices[v].active) {
                            chosenSlot = v;
                            break;
                        }
                    }
                    if (chosenSlot < 0) {
                        // All 64 voices full: steal the voice closest to finishing
                        int bestIdx = 0;
                        uint32_t maxProgress = 0;
                        for (int v = 0; v < kMaxMixVoices; ++v) {
                            uint32_t prog = voices[v].posFp;
                            if (prog >= maxProgress) {
                                maxProgress = prog;
                                bestIdx = v;
                            }
                        }
                        chosenSlot = bestIdx;
                    }
                }

                // Subtle pitch detune (253..259 around 256 = +/- 1.1%) when overlapping same sound so crowds sound rich & natural
                static const uint32_t kDetuneSteps[4] = { 256, 253, 259, 255 };
                uint32_t stepFp = (sameCount > 0) ? kDetuneSteps[(++jitterSeq) & 3] : 256u;

                voices[chosenSlot].soundId = req.soundId;
                voices[chosenSlot].pcm = &sfx.pcm;
                voices[chosenSlot].posFp = 0;
                voices[chosenSlot].stepFp = stepFp;
                voices[chosenSlot].gainL = gL;
                voices[chosenSlot].gainR = gR;
                voices[chosenSlot].startMs = nowMs;
                voices[chosenSlot].active = true;
            }
            localReqs.clear();

            // Mix all active voices (up to 64 simultaneous sounds)
            bool anyActive = false;
            memset(mixAccum, 0, sizeof(mixAccum));

            for (int v = 0; v < kMaxMixVoices; ++v) {
                SMixVoice& vc = voices[v];
                if (!vc.active || !vc.pcm || vc.pcm->empty()) continue;
                anyActive = true;

                const int16_t* src = vc.pcm->data();
                const uint32_t totalSamples = (uint32_t)vc.pcm->size();
                const uint32_t endFp = totalSamples << 8;
                uint32_t curFp = vc.posFp;
                const uint32_t stepFp = vc.stepFp;
                const int32_t gL = vc.gainL;
                const int32_t gR = vc.gainR;

                for (int f = 0; f < kMixSliceFrames; ++f) {
                    if (curFp >= endFp) {
                        vc.active = false;
                        break;
                    }
                    uint32_t idx = curFp >> 8;
                    int32_t s = src[idx];
                    mixAccum[f * 2 + 0] += (s * gL) >> 8;
                    mixAccum[f * 2 + 1] += (s * gR) >> 8;
                    curFp += stepFp;
                }
                vc.posFp = curFp;
                if (curFp >= endFp) vc.active = false;
            }

            if (anyActive) {
                if ((nowMs - lastActiveVoiceMs) >= 1500 && mixTrack && playMid) {
                    env->CallVoidMethod(mixTrack, playMid);
                    env->ExceptionClear();
                }
                lastActiveVoiceMs = nowMs;
                for (int i = 0; i < kMixSliceFrames * 2; ++i) {
                    int32_t x = mixAccum[i];
                    // Smooth soft-knee limiter prevents harsh clipping when many actors' sounds collide
                    if (x > 24576) {
                        int32_t excess = x - 24576;
                        x = 24576 + (excess * 8191) / (8191 + excess);
                    } else if (x < -24576) {
                        int32_t excess = -24576 - x;
                        x = -24576 - (excess * 8191) / (8191 + excess);
                    }
                    stereoOut[i] = (int16_t)x;
                }
                if (mixTrack && writeMid) {
                    env->SetShortArrayRegion(mixJArray, 0, kMixSliceFrames * 2, stereoOut);
                    env->CallIntMethod(mixTrack, writeMid, mixJArray, 0, kMixSliceFrames * 2);
                    env->ExceptionClear();
                }
            } else if ((nowMs - lastActiveVoiceMs) < 1500 && mixTrack && writeMid) {
                // Keep AudioTrack warm with silence for 1.5s after combat so new sounds have 0ms wake-up latency
                memset(stereoOut, 0, sizeof(stereoOut));
                env->SetShortArrayRegion(mixJArray, 0, kMixSliceFrames * 2, stereoOut);
                env->CallIntMethod(mixTrack, writeMid, mixJArray, 0, kMixSliceFrames * 2);
                env->ExceptionClear();
            } else {
                usleep(4000);
            }
        }
        return nullptr;
#endif
    }
} // anonymous namespace

static void AndroidPlayAudioInternal(const char* c_szFileName, bool isLoopMusic, float volume, float pan) {
#ifndef __APPLE__
    if (!g_pJavaVM) return;
#endif
    if (!c_szFileName || !*c_szFileName) return;

    if (volume <= 0.001f) {
        if (isLoopMusic) AndroidSetMusicVolume(0.0f);
        return;
    }
    if (volume > 1.0f) volume = 1.0f;
    if (pan < -1.0f) pan = -1.0f;
    else if (pan > 1.0f) pan = 1.0f;

    std::string origPath = c_szFileName;
    for (char& c : origPath) if (c == '\\') c = '/';
    std::string lowerPath = origPath;
    for (char& c : lowerPath) c = (char)tolower((unsigned char)c);

    const std::string baseDir = std::string(GetMobileBaseDir()) + "/";
    float leftVol  = volume * (pan > 0.0f ? (1.0f - pan * 0.75f) : 1.0f);
    float rightVol = volume * (pan < 0.0f ? (1.0f + pan * 0.75f) : 1.0f);

    if (!isLoopMusic) {
        if (!s_bSfxThreadsStarted) {
            s_bSfxThreadsStarted = true;
            pthread_create(&s_sfxMixerThread, nullptr, MobileSfxMixerThreadProc, nullptr);
            pthread_detach(s_sfxMixerThread);
#if !defined(__APPLE__)
            pthread_create(&s_sfxCompThread, nullptr, AndroidCompressedSfxThreadProc, nullptr);
            pthread_detach(s_sfxCompThread);
#endif
        }

        // GLThread-local registry: once a sound path is registered, GLThread does ZERO pack/disk lookups!
        static std::unordered_map<std::string, int> s_glSoundRegistry;
        static int s_nextSoundId = 1;

        auto itReg = s_glSoundRegistry.find(lowerPath);
        if (itReg != s_glSoundRegistry.end()) {
            int soundId = itReg->second;
            if (soundId <= 0) return; // Missing sound file cached as -1

            SSfxPlayReq req;
            req.soundId = soundId;
            req.volume = volume;
            req.pan = pan;
            req.leftVol = leftVol;
            req.rightVol = rightVol;

            pthread_mutex_lock(&s_sfxQueueMutex);
            if (s_sfxQueue.size() < 96) {
                s_sfxQueue.push_back(std::move(req));
            }
            pthread_mutex_unlock(&s_sfxQueueMutex);
            return;
        }

        // First time this sound path is ever played: read raw bytes from EterPack in memory
        std::string flatRel = "temp/";
        for (char c : lowerPath) flatRel += (isalnum((unsigned char)c) || c == '.') ? c : '_';
        std::string flatAbs = baseDir + flatRel;

        std::vector<uint8_t> rawPackBytes;
        bool existsOnDisk = (access(flatAbs.c_str(), F_OK) == 0);
        if (!existsOnDisk) {
            CMappedFile file;
            LPCVOID pData = nullptr;
            std::string dPath = "d:/ymir work/" + lowerPath;
            std::string strippedPath = lowerPath;
            if (strippedPath.compare(0, 13, "d:/ymir work/") == 0)
                strippedPath = strippedPath.substr(13);
            if (CEterPackManager::Instance().Get(file, c_szFileName, &pData) ||
                CEterPackManager::Instance().Get(file, lowerPath.c_str(), &pData) ||
                CEterPackManager::Instance().Get(file, dPath.c_str(), &pData) ||
                CEterPackManager::Instance().Get(file, strippedPath.c_str(), &pData)) {
                if (pData && file.Size() > 0 && file.Size() < (4 * 1024 * 1024)) {
                    rawPackBytes.assign((const uint8_t*)pData, (const uint8_t*)pData + file.Size());
                }
            }
        }

        if (!existsOnDisk && rawPackBytes.empty()) {
            s_glSoundRegistry[lowerPath] = -1; // Cache missing file so we never search pack again
            return;
        }

        int newId = s_nextSoundId++;
        s_glSoundRegistry[lowerPath] = newId;

        SSfxPlayReq req;
        req.soundId = newId;
        req.volume = volume;
        req.pan = pan;
        req.leftVol = leftVol;
        req.rightVol = rightVol;
        req.lowerPath = lowerPath;
        req.diskPath = std::move(flatAbs);
        req.rawPackBytes = std::move(rawPackBytes);

        pthread_mutex_lock(&s_sfxQueueMutex);
        if (s_sfxQueue.size() < 96) {
            s_sfxQueue.push_back(std::move(req));
        }
        pthread_mutex_unlock(&s_sfxQueueMutex);
        return;
    }

    // Background Music path (Android & iOS)
    static std::unordered_map<std::string, std::string> s_bgmResolvedPaths;
    std::string diskPath;
    auto itBgm = s_bgmResolvedPaths.find(lowerPath);
    if (itBgm != s_bgmResolvedPaths.end()) {
        diskPath = itBgm->second;
    } else {
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
                if (pData && file.Size() > 0 && file.Size() < (48 * 1024 * 1024)) {
                    if (FILE* fp = fopen(flatAbs.c_str(), "wb")) {
                        fwrite(pData, 1, file.Size(), fp);
                        fclose(fp);
                        diskPath = flatAbs;
                    }
                }
            }
        }
        s_bgmResolvedPaths[lowerPath] = diskPath;
    }
    if (diskPath.empty()) return;

#ifdef __APPLE__
    IOSPlayAudioFile(diskPath.c_str(), true, volume);
    return;
#else
    JNIEnv* env = nullptr;
    if (g_pJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK || !env) return;

    if (s_bgmPlayer && s_curBgmPath == diskPath) {
        AndroidSetMusicVolume(volume);
        return;
    }
    if (s_bgmPlayer) {
        AndroidStopMusic();
    }
    jclass mpCls = env->FindClass("android/media/MediaPlayer");
    jclass fisCls = env->FindClass("java/io/FileInputStream");
    if (!mpCls || !fisCls) {
        env->ExceptionClear();
        if (mpCls) env->DeleteLocalRef(mpCls);
        if (fisCls) env->DeleteLocalRef(fisCls);
        return;
    }
    jobject mp = env->NewObject(mpCls, env->GetMethodID(mpCls, "<init>", "()V"));
    if (env->ExceptionCheck() || !mp) {
        env->ExceptionClear();
        env->DeleteLocalRef(fisCls);
        env->DeleteLocalRef(mpCls);
        return;
    }

    jstring jpath = env->NewStringUTF(diskPath.c_str());
    jmethodID fisCtor = env->GetMethodID(fisCls, "<init>", "(Ljava/lang/String;)V");
    jmethodID getFdMid = env->GetMethodID(fisCls, "getFD", "()Ljava/io/FileDescriptor;");
    jmethodID closeFisMid = env->GetMethodID(fisCls, "close", "()V");
    jobject fisObj = fisCtor ? env->NewObject(fisCls, fisCtor, jpath) : nullptr;
    bool err = env->ExceptionCheck() || !fisObj;
    env->ExceptionClear();

    if (!err && fisObj) {
        jobject fdObj = env->CallObjectMethod(fisObj, getFdMid);
        err = env->ExceptionCheck() || !fdObj;
        env->ExceptionClear();
        if (!err && fdObj) {
            jmethodID setDsFdMid = env->GetMethodID(mpCls, "setDataSource", "(Ljava/io/FileDescriptor;)V");
            env->CallVoidMethod(mp, setDsFdMid, fdObj);
            err = env->ExceptionCheck();
            env->ExceptionClear();
            env->DeleteLocalRef(fdObj);
        }
        env->CallVoidMethod(fisObj, closeFisMid);
        env->ExceptionClear();
        env->DeleteLocalRef(fisObj);
    } else {
        env->CallVoidMethod(mp, env->GetMethodID(mpCls, "setDataSource", "(Ljava/lang/String;)V"), jpath);
        err = env->ExceptionCheck();
        env->ExceptionClear();
    }
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
    env->DeleteLocalRef(fisCls);
    env->DeleteLocalRef(mpCls);
#endif
}

void AndroidPlayAudioFile(const char* c_szFileName, bool isLoopMusic, float volume) {
    AndroidPlayAudioInternal(c_szFileName, isLoopMusic, volume, 0.0f);
}

void AndroidPlayAudioFile3D(const char* c_szFileName, float volume, float pan) {
    AndroidPlayAudioInternal(c_szFileName, false, volume, pan);
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