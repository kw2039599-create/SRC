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
                            if (utf) env->ReleaseStringChars(pathStr, (const jchar*)utf);
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

JNIEXPORT void JNICALL Java_com_worldonline_client_NativeLib_init(JNIEnv* env, jobject obj, jobject assetManager, jint width, jint height) {
    if (s_bEngineReady) {
        if (width > 0 && height > 0) {
            g_AndroidPhysWidth = (int)width;
            g_AndroidPhysHeight = (int)height;
            glViewport(0, 0, (GLsizei)width, (GLsizei)height);
        }
        if (IDirect3DDevice8* dev = GetCurrentGLDevice()) {
            dev->m_pCurrentRenderTargetTex = NULL;
            for (int i = 0; i < 8; ++i) dev->m_BoundGLTexId[i] = 0xFFFFFFFFu;
            dev->m_DirtyFlags = 0xFFFFFFFFu;
        }
        LOGI("WorldOnline surface resized (%dx%d) — keeping active session (ctxGen=%u)", width, height, g_AndroidGLContextGen);
        return;
    }

    LOGI("Initializing WorldOnline Engine (%dx%d)", width, height);
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

    if (width > 0 && height > 0 && access("worldonline.cfg", F_OK) != 0) {
        if (FILE* cfgFp = fopen("worldonline.cfg", "wb")) {
            fprintf(cfgFp, "WIDTH\t\t\t\t\t\t%d\nHEIGHT\t\t\t\t\t\t%d\nBPP\t\t\t\t\t\t32\nFREQUENCY\t\t\t\t\t60\nSOFTWARE_CURSOR\t\t\t0\nSOFTWARE_TILING\t\t\t0\nOBJECT_CULLING\t\t\t\t1\nVISIBILITY\t\t\t\t\t3\nMUSIC_VOLUME\t\t\t\t0.000\nVOICE_VOLUME\t\t\t\t0\nGAMMA\t\t\t\t\t\t3\nIS_SAVE_ID\t\t\t\t\t0\nSAVE_ID\t\t\t\t\t0\nDECOMPRESSED_TEXTURE\t\t0\nWINDOWED\t\t\t\t\t1\n", (int)width, (int)height);
            fclose(cfgFp);
        }
    }
	if (access("audio_init_v1.flag", F_OK) != 0) {
        if (FILE* f = fopen("audio_init_v1.flag", "wb")) fclose(f);
        if (FILE* cfgFp = fopen("worldonline.cfg", "rb")) {
            char buf[2048] = {0};
            size_t n = fread(buf, 1, sizeof(buf) - 1, cfgFp);
            fclose(cfgFp);
            std::string s(buf, n);
            size_t posM = s.find("MUSIC_VOLUME\t\t\t\t\t0.000");
            if (posM != std::string::npos) s.replace(posM, 23, "MUSIC_VOLUME\t\t\t\t\t0.500");
            size_t posV = s.find("VOICE_VOLUME\t\t\t\t\t0");
            if (posV != std::string::npos) s.replace(posV, 19, "VOICE_VOLUME\t\t\t\t\t3");
            if (FILE* outFp = fopen("worldonline.cfg", "wb")) {
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
    Tracen("worldonline Mobile: Log initialized [BUILD v5 - IOS FIX].");

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
    Tracen("worldonline Mobile: PackInitialize('pack') succeeded.");

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

    Tracen("worldonline Mobile: Starting CPythonLauncher...");
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
        initmobileInput(); PyEval_SetProfile(MobilePyProfileTracer, NULL); g_szCrashStage = "Init: RunMainScript (system.py)";
#endif
        Tracen("worldonline Mobile: Running RunMainScript (system.py)...");
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
    Tracen("worldonline Mobile: Engine Initialized Successfully!");
    LOGI("worldonline Mobile Engine Initialized Successfully");
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
                float s = outerR / 56.0f;
                // 1. Cardinal diamond notches on the outer bezel + inner runic combat ring
                float notchX = fabsf(px), notchY = fabsf(py);
                float cardD = fminf(hypotf(notchX, notchY - (outerR - rimW * 0.5f)),
                                    hypotf(notchX - (outerR - rimW * 0.5f), notchY));
                if (cardD < 4.2f * s)
                    iconMask = fmaxf(iconMask, (1.0f - SmoothStepF(2.2f * s, 4.2f * s, cardD)) * 0.90f);

                float runeRing = fabsf(d - 38.0f * s);
                if (runeRing < 1.5f * s) {
                    float dash = sinf(atan2f(py, px) * 12.0f);
                    if (dash > -0.2f)
                        iconMask = fmaxf(iconMask, (1.0f - runeRing / (1.5f * s)) * 0.42f);
                }

                // 2. Two crossed tapered broadswords (45-deg and 135-deg)
                auto EvalSword = [&](float sx, float sy) -> float {
                    // Rotate into sword-local space where u is along the blade (-26..+28) and v is perpendicular
                    float m = 0.0f;
                    // Tapered broadsword blade (u from -6 to +27)
                    if (sx >= -6.0f * s && sx <= 27.5f * s) {
                        float t = (sx + 6.0f * s) / (33.5f * s); // 0 at guard, 1 at tip
                        float halfW = (t < 0.78f)
                            ? (3.6f * s + t * 0.6f * s)
                            : ((1.0f - (t - 0.78f) / 0.22f) * 4.1f * s);
                        float absV = fabsf(sy);
                        if (absV <= halfW + 1.0f * s) {
                            float blade = 1.0f - SmoothStepF(fmaxf(0.0f, halfW - 0.9f * s), halfW + 0.9f * s, absV);
                            // Central fuller (blood groove) for 3D metallic depth
                            if (t > 0.06f && t < 0.72f && absV < 0.95f * s)
                                blade *= 0.38f;
                            m = fmaxf(m, blade);
                        }
                    }
                    // Flared winged crossguard (u around -7.5, v from -13 to +13)
                    float guardCurve = -7.2f * s + (sy * sy) / (42.0f * s);
                    float gDist = fabsf(sx - guardCurve);
                    if (fabsf(sy) <= 13.5f * s) {
                        float gThick = (2.4f - 0.08f * (fabsf(sy) / s)) * s;
                        if (gDist <= gThick + 0.8f * s)
                            m = fmaxf(m, 1.0f - SmoothStepF(fmaxf(0.0f, gThick - 0.6f * s), gThick + 0.8f * s, gDist));
                    }
                    // Wrapped hilt grip (u from -18.5 to -8)
                    if (sx >= -18.5f * s && sx <= -7.5f * s && fabsf(sy) <= 2.3f * s) {
                        m = fmaxf(m, 1.0f - SmoothStepF(1.4f * s, 2.3f * s, fabsf(sy)));
                    }
                    // Faceted round pommel (centered at u = -21.5, v = 0)
                    float pomD = hypotf(sx + 21.5f * s, sy);
                    if (pomD <= 4.2f * s) {
                        m = fmaxf(m, 1.0f - SmoothStepF(2.8f * s, 4.2f * s, pomD));
                    }
                    return m;
                };

                // Sword 1: bottom-left to top-right
                float s1u = (px - py) * 0.70710678f;
                float s1v = (px + py) * 0.70710678f;
                // Sword 2: bottom-right to top-left
                float s2u = (-px - py) * 0.70710678f;
                float s2v = (px - py) * 0.70710678f;

                float sw1 = EvalSword(s1u, s1v);
                float sw2 = EvalSword(s2u, s2v);
                // Slight shadow gap where Sword 1 crosses over Sword 2
                if (sw1 > 0.1f) {
                    iconMask = fmaxf(iconMask, sw1);
                } else if (sw2 > 0.1f) {
                    float underShadow = (fabsf(s1u) < 5.2f * s && fabsf(s1v) < 5.8f * s) ? 0.55f : 1.0f;
                    iconMask = fmaxf(iconMask, sw2 * underShadow);
                }
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
                // Open Hand silhouette (palm + thumb + 4 fingers)
                float ux = px / (outerR / 28.0f);
                float uy = py / (outerR / 28.0f);
                float palmD = hypotf(ux * 0.92f, (uy - 4.2f) * 1.05f) - 6.2f;
                float wristBox = fmaxf(fabsf(ux) - 4.2f, fabsf(uy - 8.5f) - 2.8f);
                float hDist = fminf(palmD, wristBox);
                // Thumb (left) + 4 fingers (index, middle, ring, pinky)
                hDist = fminf(hDist, DistToSegF(ux, uy, -4.5f,  4.5f, -10.5f, -1.5f) - 1.9f);
                hDist = fminf(hDist, DistToSegF(ux, uy, -4.2f,  0.5f,  -5.4f, -9.5f) - 1.65f);
                hDist = fminf(hDist, DistToSegF(ux, uy, -1.4f, -0.5f,  -1.6f,-11.8f) - 1.70f);
                hDist = fminf(hDist, DistToSegF(ux, uy,  1.6f,  0.0f,   2.2f,-10.5f) - 1.65f);
                hDist = fminf(hDist, DistToSegF(ux, uy,  4.3f,  1.5f,   5.6f, -7.2f) - 1.50f);
                if (hDist < 0.8f) {
                    iconMask = 1.0f - SmoothStepF(-0.5f, 0.8f, hDist);
                    // Subtle crease line across palm for depth
                    float crease = DistToSegF(ux, uy, -3.5f, 2.2f, 3.8f, 3.2f);
                    if (crease < 0.7f) iconMask *= 0.45f;
                }
            } else if (iconType == MICON_PAGE) {
                float s = outerR / 28.0f;
                float orbD = fabsf(d - 12.5f * s);
                if (orbD < 2.0f * s && fabsf(px) > 2.5f * s)
                    iconMask = 1.0f - SmoothStepF(0.7f * s, 2.0f * s, orbD);
            } else if (iconType == MICON_MOUNT) {
                // Stylized Horse Head & Neck silhouette (facing right)
                float ux = px / (outerR / 28.0f);
                float uy = py / (outerR / 28.0f);
                float hDist = 100.0f;
                // Base pedestal + arched neck + cranium + tapered muzzle + ears + mane
                hDist = fminf(hDist, DistToSegF(ux, uy, -6.5f, 11.2f,  4.5f, 11.2f) - 2.0f);
                hDist = fminf(hDist, DistToSegF(ux, uy, -3.2f,  9.5f, -4.2f, -1.5f) - 4.4f);
                hDist = fminf(hDist, DistToSegF(ux, uy, -4.2f, -1.5f, -1.5f, -7.5f) - 4.0f);
                hDist = fminf(hDist, hypotf(ux + 1.2f, uy + 6.5f) - 4.5f);
                hDist = fminf(hDist, DistToSegF(ux, uy,  0.0f, -6.8f,  9.2f, -2.5f) - 3.1f);
                hDist = fminf(hDist, DistToSegF(ux, uy, -1.0f, -3.8f,  6.5f, -1.5f) - 2.8f);
                // Pointed horse ears at crown
                hDist = fminf(hDist, DistToSegF(ux, uy, -2.2f, -9.0f, -0.5f,-13.8f) - 1.35f);
                hDist = fminf(hDist, DistToSegF(ux, uy, -4.2f, -8.2f, -3.5f,-12.8f) - 1.20f);
                // Flowing mane ridges along back of neck
                hDist = fminf(hDist, DistToSegF(ux, uy, -6.2f, -6.5f, -8.5f,  2.5f) - 2.2f);
                hDist = fminf(hDist, DistToSegF(ux, uy, -8.2f,  1.5f, -7.2f,  8.5f) - 2.2f);
                if (hDist < 0.8f) {
                    iconMask = 1.0f - SmoothStepF(-0.5f, 0.8f, hDist);
                    // Carve out eye and nostril for crisp horse detail
                    if (hypotf(ux - 1.2f, uy + 6.8f) < 1.15f || hypotf(ux - 7.8f, uy + 2.6f) < 0.85f)
                        iconMask *= 0.20f;
                }
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
                        // Native worldonline Dark Iron & Pewter Bevel Frame (matching 67Untitled.png)
                        if (d == 0) {
                            pix[i+0] = 22; pix[i+1] = 24; pix[i+2] = 26; pix[i+3] = 245;
                        } else if (d == 1) {
                            float grad = 0.80f + 0.20f * (1.0f - (float)(x + y) / (2.0f * sz));
                            pix[i+0] = ClampU8(152.0f * grad);
                            pix[i+1] = ClampU8(162.0f * grad);
                            pix[i+2] = ClampU8(170.0f * grad);
                            pix[i+3] = 255;
                        } else if (d == 2 || d == 3) {
                            float grad = 0.75f + 0.25f * (1.0f - (float)(x + y) / (2.0f * sz));
                            pix[i+0] = ClampU8(116.0f * grad);
                            pix[i+1] = ClampU8(126.0f * grad);
                            pix[i+2] = ClampU8(134.0f * grad);
                            pix[i+3] = 250;
                        } else if (d == 4) {
                            pix[i+0] = 32; pix[i+1] = 35; pix[i+2] = 38; pix[i+3] = 235;
                        } else if (d == 5) {
                            pix[i+0] = 74; pix[i+1] = 82; pix[i+2] = 88; pix[i+3] = 190;
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
    float s = size / 40.0f;
    size_t idx = 0;

    for (int y = 0; y < size; ++y) {
        float py = ((y + 0.5f) - half) / s;
        for (int x = 0; x < size; ++x) {
            float px = ((x + 0.5f) - half) / s;
            float ax = fabsf(px), ay = fabsf(py);
            float d = hypotf(px, py);

            // Soft, transparent dark-glasstint aura (no heavy box border, matching Royal2 top-right icons)
            float r = 12.0f, g = 10.0f, b = 8.0f, a = 0.0f;
            if (d < 18.5f) {
                float radial = 1.0f - SmoothStepF(6.0f, 18.5f, d);
                a = radial * (isPressed ? 145.0f : 68.0f);
                if (isPressed) {
                    r = 45.0f; g = 38.0f; b = 28.0f;
                }
            }

            // Crisp warm-ivory vector glyph (scaled up for high visibility)
            float ic = 0.0f;
            if (iconType == TICON_COLLAPSE) {
                // Sleek '>' wing arrow
                float d1 = DistToSegF(px, py, -5.0f, -6.5f, 5.5f, 0.0f);
                float d2 = DistToSegF(px, py, 5.5f, 0.0f, -5.0f, 6.5f);
                float d3 = DistToSegF(px, py, -1.5f, -3.5f, 5.5f, 0.0f);
                float d4 = DistToSegF(px, py, 5.5f, 0.0f, -1.5f, 3.5f);
                float md = fminf(fminf(d1, d2), fminf(d3, d4));
                if (md < 2.2f) ic = 1.0f - SmoothStepF(0.9f, 2.2f, md);
            } else if (iconType == TICON_EXPAND) {
                // Sleek '<' wing arrow
                float d1 = DistToSegF(px, py, 5.0f, -6.5f, -5.5f, 0.0f);
                float d2 = DistToSegF(px, py, -5.5f, 0.0f, 5.0f, 6.5f);
                float md = fminf(d1, d2);
                if (md < 2.2f) ic = 1.0f - SmoothStepF(0.9f, 2.2f, md);
            } else if (iconType == TICON_SYS) {
                // Hamburger Menu (3 crisp horizontal bars)
                if (ax < 8.5f && (fabsf(py + 6.0f) < 1.45f || fabsf(py) < 1.45f || fabsf(py - 6.0f) < 1.45f))
                    ic = 0.98f;
            } else if (iconType == TICON_CHAR) {
                // Spartan / Warrior Helmet silhouette (matching reference image)
                float dome = hypotf(px, py + 1.0f);
                if (dome < 7.8f && py < 5.5f) {
                    if (!(fabsf(px) < 2.2f && py > -1.0f) && !(py > -2.5f && py < -0.5f && fabsf(px) >= 1.5f && fabsf(px) <= 5.2f))
                        ic = 1.0f - SmoothStepF(6.8f, 7.8f, dome);
                }
            } else if (iconType == TICON_INVEN) {
                // Treasure Chest silhouette
                if (ax < 7.8f && py >= -5.5f && py <= 6.8f) {
                    if (fabsf(py + 1.0f) > 0.9f && !(ax < 1.6f && fabsf(py + 1.0f) < 2.4f))
                        ic = 0.96f;
                }
            } else if (iconType == TICON_COMM) {
                // Community / 2 People silhouette
                float h1 = hypotf(px + 2.8f, py + 4.2f);
                float h2 = hypotf(px - 4.2f, py + 3.0f);
                if (h1 < 3.1f || h2 < 2.6f) ic = 0.98f;
                if (py > 0.5f && py < 7.8f) {
                    if (hypotf((px + 2.8f) * 0.78f, py - 4.8f) < 4.8f || hypotf((px - 4.2f) * 0.82f, py - 5.2f) < 4.0f)
                        ic = 0.98f;
                }
            } else if (iconType == TICON_SHOP) {
                // Scroll / Market List with checkmark
                if (ax < 6.8f && py >= -7.2f && py <= 7.2f) {
                    float border = fmaxf(ax - 5.2f, fabsf(py) - 5.6f);
                    float chk1 = DistToSegF(px, py, -3.0f, 0.5f, -0.5f, 3.0f);
                    float chk2 = DistToSegF(px, py, -0.5f, 3.0f, 3.8f, -2.5f);
                    if (border > -1.6f || fminf(chk1, chk2) < 1.5f)
                        ic = 0.96f;
                }
            } else if (iconType == TICON_AUTOHUNT) {
                // Crossed Swords
                float d1 = DistToSegF(px, py, -7.2f, 7.2f, 7.2f, -7.2f);
                float d2 = DistToSegF(px, py, 7.2f, 7.2f, -7.2f, -7.2f);
                float g1 = DistToSegF(px, py, -7.2f, 2.2f, -2.2f, 7.2f);
                float g2 = DistToSegF(px, py, 2.2f, 7.2f, 7.2f, 2.2f);
                float md = fminf(fminf(d1, d2), fminf(g1, g2) * 1.15f);
                if (md < 1.9f) ic = 1.0f - SmoothStepF(0.8f, 1.9f, md);
            } else if (iconType == TICON_DS) {
                // Dragon Soul Faceted Diamond
                float dia = ax * 1.12f + ay * 0.85f;
                if (dia < 8.4f && (dia > 5.8f || dia < 3.4f))
                    ic = 1.0f - SmoothStepF(7.6f, 8.4f, dia);
            } else if (iconType == TICON_UTIL) {
                // Bell / Utility Icon (matching reference top bar)
                float bellW = (py >= -5.5f && py <= 4.2f) ? (3.2f + (py + 5.5f) * 0.45f) : 0.0f;
                if (bellW > 0.0f && ax <= bellW) ic = 1.0f - SmoothStepF(bellW - 1.0f, bellW, ax);
                if (fabsf(py - 4.5f) < 1.2f && ax < 8.2f) ic = 0.98f;
                if (hypotf(px, py + 6.5f) < 1.8f || hypotf(px, py - 6.8f) < 2.2f) ic = fmaxf(ic, 0.96f);
            } else if (iconType == TICON_SPEC_BOXES) {
                // Two stacked/side-by-side storage boxes icon for Special Storage
                float b1x = px + 3.2f, b1y = py + 2.6f; // Back-left box
                float b2x = px - 2.8f, b2y = py - 2.4f; // Front-right box
                bool inFront = (fabsf(b2x) < 5.6f && fabsf(b2y) < 4.8f);
                bool inBack  = (fabsf(b1x) < 5.4f && fabsf(b1y) < 4.6f) && !(fabsf(b2x) < 6.4f && fabsf(b2y) < 5.6f);
                if (inFront) {
                    if (fabsf(b2y + 0.8f) > 0.75f && !(fabsf(b2x) < 1.3f && fabsf(b2y + 0.8f) < 1.8f))
                        ic = 0.98f;
                } else if (inBack) {
                    if (fabsf(b1y + 0.8f) > 0.75f)
                        ic = 0.88f;
                }
            }

            if (ic > 0.0f) {
                float icR = isPressed ? 255.0f : 244.0f;
                float icG = isPressed ? 225.0f : 239.0f;
                float icB = isPressed ? 155.0f : 228.0f;
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
    float outerR = 12.2f;
    float innerR = 10.2f;

    for (int y = 0; y < size; ++y) {
        float py = (y + 0.5f) - half;
        for (int x = 0; x < size; ++x) {
            float px = (x + 0.5f) - half;
            float d = hypotf(px, py);
            size_t i = ((size_t)y * size + (size_t)x) * 4;

            float r = 10.0f, g = 12.0f, b = 16.0f, a = 0.0f;
            if (d <= outerR + 1.2f) {
                float mask = 1.0f - SmoothStepF(outerR - 0.6f, outerR + 1.2f, d);
                if (!isFilled) {
                    // Empty recessed dark socket + classic bronze/gold rim
                    r = 16.0f; g = 18.0f; b = 22.0f;
                    a = mask * 215.0f;
                    float rimD = fabsf(d - (outerR - 1.0f));
                    if (rimD < 1.5f) {
                        float rm = 1.0f - rimD / 1.5f;
                        float grad = 0.65f + 0.35f * (1.0f - (float)(x + y) / (2.0f * size));
                        r = r * (1.0f - rm) + (185.0f * grad) * rm;
                        g = g * (1.0f - rm) + (148.0f * grad) * rm;
                        b = b * (1.0f - rm) + (82.0f * grad) * rm;
                        a = fmaxf(a, rm * 245.0f);
                    }
                } else if (d <= innerR + 0.6f) {
                    // Glowing 3D liquid sphere with top-left specular highlight
                    float fillMask = 1.0f - SmoothStepF(innerR - 0.6f, innerR + 0.6f, d);
                    float sphereZ = sqrtf(fmaxf(0.0f, 1.0f - (d * d) / (innerR * innerR)));
                    float spec = powf(fmaxf(0.0f, (-0.45f * px - 0.55f * py) / innerR + sphereZ * 0.65f), 3.0f) * 95.0f;
                    float core = 0.55f + 0.45f * sphereZ;
                    r = fminf(255.0f, cr * core + spec);
                    g = fminf(255.0f, cg * core + spec);
                    b = fminf(255.0f, cb * core + spec * 0.7f);
                    a = fillMask * 250.0f;
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
    float innerR = 23.0f;

    for (int y = 0; y < size; ++y) {
        float py = (y + 0.5f) - half;
        for (int x = 0; x < size; ++x) {
            float px = (x + 0.5f) - half;
            float d = hypotf(px, py);
            size_t i = ((size_t)y * size + (size_t)x) * 4;

            float r = 18.0f, g = 18.0f, b = 20.0f, a = 0.0f;

            if (d >= innerR && d < outerR + 2.5f) {
                float sh = 1.0f - SmoothStepF(outerR - 0.5f, outerR + 2.5f, d);
                a = sh * 135.0f;
            }
            if (d >= innerR - 1.0f && d <= outerR) {
                float inMask  = SmoothStepF(innerR - 1.0f, innerR + 0.8f, d);
                float outMask = 1.0f - SmoothStepF(outerR - 1.0f, outerR + 0.4f, d);
                float ringMask = inMask * outMask;

                float grad = 0.68f + 0.32f * (1.0f - (float)(x + y) / (2.0f * size));
                if (d < innerR + 1.5f || d > outerR - 1.5f) {
                    // Pewter / Iron outer & inner bevel ridge
                    r = 162.0f * grad; g = 156.0f * grad; b = 145.0f * grad;
                } else {
                    // Native worldonline dark copper-brown / iron core
                    r = 108.0f * grad; g = 58.0f * grad; b = 32.0f * grad;
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
            if (x >= w - 2 && (y < 2 || y >= h - 2)) continue;

            bool isOuterDark = (y == 0 || y == h - 1 || x == 0 || x == w - 1);
            bool isPewterBevel = (y == 1 || y == 2 || y == h - 2 || x == 1 || x == w - 2);
            bool isInnerShadow = (y == 3 || y == h - 3 || x == w - 3);
            bool isDivider = (y == 17 || y == 18 || y == 30 || y == 31) && (x >= 18 && x <= w - 4);
            float r = 18.0f, g = 18.0f, b = 20.0f, a = 242.0f;

            if (isOuterDark) {
                r = 34.0f; g = 32.0f; b = 30.0f; a = 250.0f;
            } else if (isPewterBevel) {
                float grad = (y == 1) ? 1.12f : (0.78f + 0.22f * (1.0f - (float)y / (float)h));
                r = 148.0f * grad; g = 142.0f * grad; b = 132.0f * grad; a = 255.0f;
            } else if (isInnerShadow) {
                r = 46.0f; g = 43.0f; b = 40.0f; a = 248.0f;
            } else if (isDivider) {
                float divGrad = (y == 17 || y == 30) ? 1.0f : 0.55f;
                r = 128.0f * divGrad; g = 122.0f * divGrad; b = 114.0f * divGrad; a = 245.0f;
            } else {
                // Recessed channels extend full length x: 22..192 (171px) right up to the right inner shadow (w - 3)
                bool inSlot = (y >= 4 && y <= 16) || (y >= 19 && y <= 29) || (y >= 32 && y <= 39);
                if (inSlot && x >= 22 && x <= w - 4) {
                    r = 10.0f; g = 10.0f; b = 12.0f; a = 248.0f;
                } else {
                    r = 26.0f; g = 25.0f; b = 24.0f; a = 245.0f;
                }
            }
            pix[i + 0] = ClampU8(b); pix[i + 1] = ClampU8(g); pix[i + 2] = ClampU8(r); pix[i + 3] = ClampU8(a);
        }
    }
    fwrite(pix.data(), 1, pix.size(), fp);
    fclose(fp);
}

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
        float ny = (float)y / (float)(h > 1 ? h - 1 : 1); // 0 = top, 1 = bottom
        float cyl = 0.56f + 0.44f * sinf(ny * 3.14159265f);
        float topSpec = (ny >= 0.08f && ny <= 0.34f) ? 58.0f : 0.0f;
        for (int x = 0; x < w; ++x) {
            size_t i = ((size_t)y * w + (size_t)x) * 4;
            // Flowing liquid wave + diagonal shimmer band that loops seamlessly across 8 frames
            float waveAngle = ((float)x / 42.75f + ny * 0.35f) * 6.2831853f - phase;
            float wave = 0.5f + 0.5f * sinf(waveAngle);
            float shimmer = powf(wave, 3.0f) * 42.0f * sinf(ny * 3.14159265f);
            float bodyBoost = 0.90f + 0.18f * wave;
            float r = fminf(255.0f, cr * cyl * bodyBoost + topSpec + shimmer);
            float g = fminf(255.0f, cg * cyl * bodyBoost + topSpec + shimmer);
            float b = fminf(255.0f, cb * cyl * bodyBoost + topSpec * 0.85f + shimmer * 0.9f);
            pix[i + 0] = ClampU8(b); pix[i + 1] = ClampU8(g); pix[i + 2] = ClampU8(r); pix[i + 3] = 252;
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
                // Upward Equip/Use Sword & Arrow emblem
                float dBlade = DistToSegF(px, py, -7.0f, 7.0f, 7.5f, -7.5f);
                float dGuard = DistToSegF(px, py, -6.5f, 0.5f, -0.5f, 6.5f);
                if (fminf(dBlade, dGuard) < 1.8f) ic = 1.0f - SmoothStepF(0.8f, 1.8f, fminf(dBlade, dGuard));
            } else if (iconType == TTICON_SPLIT) {
                // Split stacks icon: two separated boxes + outward arrows
                bool boxL = (px >= -10.5f && px <= -3.5f && ay <= 4.5f);
                bool boxR = (px >= 3.5f && px <= 10.5f && ay <= 4.5f);
                float sepLine = (ax < 1.1f && ay < 8.5f) ? 0.95f : 0.0f;
                if (boxL || boxR) {
                    float bEdge = fminf(fabsf(ax - 7.0f) - 2.2f, ay - 3.2f);
                    if (bEdge > -1.4f) ic = 0.96f;
                }
                ic = fmaxf(ic, sepLine);
            } else if (iconType == TTICON_LINK) {
                // Chat Speech Bubble + Link node
                float bd = hypotf(px / 9.5f, (py + 1.5f) / 7.2f);
                if (bd <= 1.0f && bd >= 0.72f) ic = 0.96f;
                if (fabsf(py + 1.5f) < 1.4f && (fabsf(px) < 1.4f || fabsf(ax - 4.2f) < 1.4f)) ic = 0.96f;
                if (DistToSegF(px, py, -3.0f, 5.0f, -6.5f, 8.5f) < 1.6f) ic = 0.96f;
            } else if (iconType == TTICON_CHEST) {
                // Open Chest / Bundle icon
                if (ax < 8.5f && py >= -6.0f && py <= 6.5f) {
                    if (fabsf(py + 0.8f) > 0.9f && !(ax < 1.8f && fabsf(py + 0.8f) < 2.2f))
                        ic = 0.95f;
                }
            } else if (iconType == TTICON_PREVIEW) {
                // Eye / Preview icon
                float eyeY = 4.8f * (1.0f - (px * px) / 81.0f);
                if (ax <= 9.0f && fabsf(ay - eyeY) < 1.5f) ic = 0.96f;
                if (hypotf(px, py) < 2.8f) ic = 0.98f;
            } else if (iconType == TTICON_CLOSE) {
                // Crisp X icon
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

static void WriteMobileStatusHUDTextures() {
    WriteMobileRingTGA("mark/m_portrait_bg.tga", 64, 28.5f, 2.2f, 215.0f, 146,140,130, 175,168,156, MICON_NONE, false);
    WritePortraitRingTGA("mark/m_portrait_ring.tga");
    WriteExpSphereTGA("mark/m_exp_orb_empty_v3.tga", false, 0.0f, 0.0f, 0.0f);
    WriteExpSphereTGA("mark/m_exp_orb_full_v3.tga",  true, 225.0f, 175.0f, 35.0f);
    WriteExpSphereTGA("mark/m_exp_orb_conq_v3.tga",  true,  65.0f, 195.0f, 245.0f);
    WriteStatusPlateTGA("mark/m_hud_plate_v4.tga");
    for (int f = 0; f < 8; ++f) {
        char hpPath[64], spPath[64], enPath[64];
        snprintf(hpPath, sizeof(hpPath), "mark/m_hud_hp_v4_%d.tga", f);
        snprintf(spPath, sizeof(spPath), "mark/m_hud_sp_v4_%d.tga", f);
        snprintf(enPath, sizeof(enPath), "mark/m_hud_en_v4_%d.tga", f);
        WriteGlossyBarFillTGA(hpPath, 171, 13, 220.0f, 34.0f, 34.0f, f);
        WriteGlossyBarFillTGA(spPath, 171, 11,  34.0f, 115.0f, 225.0f, f);
        WriteGlossyBarFillTGA(enPath, 171,  8,  95.0f, 180.0f, 220.0f, f);
    }

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
}

void GenerateMobileHUDTextures() {
    // One-time migration: remove old cached TGAs so native worldonline textures regenerate cleanly
    if (access("mark/theme_wo_native_v4.flag", F_OK) != 0) {
        static const char* s_oldTgas[] = {
            "mark/m_minimap_frame.tga", "mark/m_portrait_bg.tga", "mark/m_portrait_ring.tga",
            "mark/m_hud_plate_v2.tga", "mark/m_hud_plate_v3.tga",
            "mark/m_exp_orb_empty_v3.tga", "mark/m_exp_orb_full_v3.tga", "mark/m_exp_orb_conq_v3.tga",
            "mark/m_tt_use_norm.tga", "mark/m_tt_use_down.tga", "mark/m_tt_split_norm.tga", "mark/m_tt_split_down.tga",
            "mark/m_tt_link_norm.tga", "mark/m_tt_link_down.tga", "mark/m_tt_chest_norm.tga", "mark/m_tt_chest_down.tga",
            "mark/m_tt_preview_norm.tga", "mark/m_tt_preview_down.tga", "mark/m_tt_close_norm.tga", "mark/m_tt_close_down.tga",
            "mark/m_joy_bg_v2.tga", "mark/m_joy_knob_norm_v2.tga", "mark/m_joy_knob_down_v2.tga",
            "mark/m_atk_norm_v3.tga", "mark/m_atk_down_v3.tga", "mark/m_atk_norm_v5.tga", "mark/m_atk_down_v5.tga",
            "mark/m_slot_norm_v3.tga", "mark/m_slot_over_v3.tga",
            "mark/m_btn_lock_norm_v3.tga", "mark/m_btn_lock_down_v3.tga",
            "mark/m_btn_loot_norm_v3.tga", "mark/m_btn_loot_down_v3.tga",
            "mark/m_btn_page_norm_v3.tga", "mark/m_btn_page_down_v3.tga",
            "mark/m_btn_mount_norm_v3.tga", "mark/m_btn_mount_down_v3.tga",
            "mark/m_btn_chat_norm_v3.tga", "mark/m_btn_chat_down_v3.tga"
        };
        for (const char* p : s_oldTgas) unlink(p);
        if (FILE* f = fopen("mark/theme_wo_native_v4.flag", "wb")) fclose(f);
    }

    // Native worldonline Pewter/Iron Rim (148,142,132) + Warm Copper-Brown Accent (145,78,42)
    WriteMobileRingTGA("mark/m_joy_bg_v2.tga",        160, 74.0f, 3.8f, 115.0f, 148,142,132, 185,178,166, MICON_JOY_BG,      false);
    WriteMobileRingTGA("mark/m_joy_knob_norm_v2.tga",  76, 33.0f, 3.2f, 185.0f, 148,142,132, 175,105, 62, MICON_JOY_KNOB,    false);
    WriteMobileRingTGA("mark/m_joy_knob_down_v2.tga",  76, 35.0f, 3.8f, 220.0f, 185,178,166, 205,120, 68, MICON_JOY_KNOB,    true);

    WriteSquaredMiniMapTextures();
    WriteMobileStatusHUDTextures();

    WriteTopBarButtonTGA("mark/m_top_collapse_norm_v4.tga", TICON_COLLAPSE, false);
    WriteTopBarButtonTGA("mark/m_top_collapse_down_v4.tga", TICON_COLLAPSE, true);
    WriteTopBarButtonTGA("mark/m_top_expand_norm_v4.tga",   TICON_EXPAND,   false);
    WriteTopBarButtonTGA("mark/m_top_expand_down_v4.tga",   TICON_EXPAND,   true);
    WriteTopBarButtonTGA("mark/m_top_char_norm_v4.tga",     TICON_CHAR,     false);
    WriteTopBarButtonTGA("mark/m_top_char_down_v4.tga",     TICON_CHAR,     true);
    WriteTopBarButtonTGA("mark/m_top_inven_norm_v4.tga",    TICON_INVEN,    false);
    WriteTopBarButtonTGA("mark/m_top_inven_down_v4.tga",    TICON_INVEN,    true);
    WriteTopBarButtonTGA("mark/m_top_comm_norm_v4.tga",     TICON_COMM,     false);
    WriteTopBarButtonTGA("mark/m_top_comm_down_v4.tga",     TICON_COMM,     true);
    WriteTopBarButtonTGA("mark/m_top_shop_norm_v4.tga",     TICON_SHOP,     false);
    WriteTopBarButtonTGA("mark/m_top_shop_down_v4.tga",     TICON_SHOP,     true);
    WriteTopBarButtonTGA("mark/m_top_auto_norm_v4.tga",     TICON_AUTOHUNT, false);
    WriteTopBarButtonTGA("mark/m_top_auto_down_v4.tga",     TICON_AUTOHUNT, true);
    WriteTopBarButtonTGA("mark/m_top_ds_norm_v4.tga",       TICON_DS,       false);
    WriteTopBarButtonTGA("mark/m_top_ds_down_v4.tga",       TICON_DS,       true);
    WriteTopBarButtonTGA("mark/m_top_util_norm_v4.tga",     TICON_UTIL,     false);
    WriteTopBarButtonTGA("mark/m_top_util_down_v4.tga",     TICON_UTIL,     true);
    WriteTopBarButtonTGA("mark/m_top_sys_norm_v4.tga",      TICON_SYS,      false);
    WriteTopBarButtonTGA("mark/m_top_sys_down_v4.tga",      TICON_SYS,      true);
    WriteTopBarButtonTGA("mark/m_top_spec_norm_v4.tga",     TICON_SPEC_BOXES, false);
    WriteTopBarButtonTGA("mark/m_top_spec_down_v4.tga",     TICON_SPEC_BOXES, true);

    // Native worldonline Dark Charcoal Glass + Pewter-Iron Bevel & Copper-Rust Active Glow
    // 2x HiDPI Native worldonline Dark Charcoal & Pewter-Iron Rings (stay razor-sharp when resized up to 180%)
    WriteMobileRingTGA("mark/m_atk_norm_v3.tga",       280, 128.0f, 10.4f, 200.0f, 152,146,136, 188,112, 68, MICON_ATK_SWORDS,  false);
    WriteMobileRingTGA("mark/m_atk_down_v3.tga",       280, 132.0f, 12.0f, 235.0f, 192,185,174, 215,125, 72, MICON_ATK_SWORDS,  true);
    WriteMobileRingTGA("mark/m_slot_norm_v3.tga",      144,  67.0f,  7.2f, 190.0f, 146,140,130, 175,168,156, MICON_NONE,        false);
    WriteMobileRingTGA("mark/m_slot_over_v3.tga",      144,  69.0f,  8.4f, 225.0f, 188,182,170, 195,115, 68, MICON_NONE,        true);
    WriteMobileRingTGA("mark/m_btn_lock_norm_v3.tga",  128,  54.0f,  6.8f, 195.0f, 146,140,130, 185,178,166, MICON_TARGET,      false);
    WriteMobileRingTGA("mark/m_btn_lock_down_v3.tga",  128,  56.0f,  8.0f, 230.0f, 188,182,170, 205,120, 68, MICON_TARGET,      true);
    WriteMobileRingTGA("mark/m_btn_loot_norm_v3.tga",  128,  54.0f,  6.8f, 195.0f, 146,140,130, 185,178,166, MICON_LOOT,        false);
    WriteMobileRingTGA("mark/m_btn_loot_down_v3.tga",  128,  56.0f,  8.0f, 230.0f, 188,182,170, 205,120, 68, MICON_LOOT,        true);
    WriteMobileRingTGA("mark/m_btn_page_norm_v3.tga",  128,  54.0f,  6.8f, 195.0f, 146,140,130, 185,178,166, MICON_PAGE,        false);
    WriteMobileRingTGA("mark/m_btn_page_down_v3.tga",  128,  56.0f,  8.0f, 230.0f, 188,182,170, 205,120, 68, MICON_PAGE,        true);
    WriteMobileRingTGA("mark/m_btn_mount_norm_v3.tga", 128,  57.0f,  7.2f, 195.0f, 146,140,130, 185,178,166, MICON_MOUNT,       false);
    WriteMobileRingTGA("mark/m_btn_mount_down_v3.tga", 128,  59.0f,  8.4f, 230.0f, 188,182,170, 205,120, 68, MICON_MOUNT,       true);
    WriteMobileRingTGA("mark/m_btn_chat_norm_v3.tga",  112,  49.0f,  6.4f, 195.0f, 146,140,130, 185,178,166, MICON_CHAT_BUBBLE, false);
    WriteMobileRingTGA("mark/m_btn_chat_down_v3.tga",  112,  51.0f,  7.6f, 230.0f, 188,182,170, 205,120, 68, MICON_CHAT_BUBBLE, true);
    WriteMobileRingTGA("mark/m_atk_norm_v5.tga",       288, 132.0f, 10.8f, 205.0f, 152,146,136, 192,115, 68, MICON_ATK_SWORDS,  false);
    WriteMobileRingTGA("mark/m_atk_down_v5.tga",       288, 136.0f, 12.4f, 238.0f, 195,188,176, 220,128, 72, MICON_ATK_SWORDS,  true);
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
    UI::CWindow* pHoldDragWnd = nullptr;
};

struct SScaledWindowRect {
    float scale = 1.0f;
    int x = 0, y = 0, w = 0, h = 0;
    uint32_t lastSeenFrame = 0;
};
static std::unordered_map<uintptr_t, SScaledWindowRect> s_scaledWindows;
static uint32_t s_uMobileRenderFrame = 1;
static D3DXMATRIX s_savedUiViewMat;
static bool s_bUiScaleActive = false;
static int s_iLastTouchDownX = -1000;
static int s_iLastTouchDownY = -1000;
static bool s_bLastTouchSnappedToButton = false;
static bool s_bExplicitKeyboardRequest = false;

static void MapTouchThroughScaledWindows(int inX, int inY, int* outX, int* outY) {
    *outX = inX;
    *outY = inY;
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
                *outX = r.x + (int)((inX - r.x) / r.scale + 0.5f);
                *outY = r.y + (int)((inY - r.y) / r.scale + 0.5f);
                return;
            }
        }
        ++it;
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

static UI::CWindow* FindHoldDraggableWindow(int x, int y) {
    UI::CWindowManager::Instance().RunMouseMove(x, y);
    UI::CWindow* pPointWnd = SanitizeButtonChildPickFlags(UI::CWindowManager::Instance().GetPointWindow());
    if (!pPointWnd) return nullptr;

    // Never hijack scrollbars, sliders, buttons, or editlines into a window drag!
    bool isThumb = false, isBtn = false;
    if (IsScrollOrSliderOrButtonWidget(pPointWnd, &isThumb, &isBtn)) {
        return nullptr;
    }

    int testX = x, testY = y;
    bool snapBtn = false, snapThumb = false;
    ApplySmartTouchMagnetism(x, y, &testX, &testY, &snapBtn, &snapThumb);
    if (snapThumb || snapBtn) {
        return nullptr;
    }

    UI::CWindow* pBestMovable = nullptr;
    for (UI::CWindow* pCur = pPointWnd; pCur != nullptr; pCur = pCur->GetParent()) {
        const char* szName = pCur->GetName();
        if (szName) {
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

    if (!s_pLiveEditText) {
        jclass atCls = env->FindClass("android/app/ActivityThread");
        if (!atCls) { env->ExceptionClear(); return; }
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

        if (activityObj) {
            jclass actCls = env->FindClass("android/app/Activity");
            jmethodID findMid = env->GetMethodID(actCls, "findViewById", "(I)Landroid/view/View;");
            jobject contentView = findMid ? env->CallObjectMethod(activityObj, findMid, (jint)16908290) : nullptr; // android.R.id.content
            env->ExceptionClear();

            if (contentView) {
                jclass etCls = env->FindClass("android/widget/EditText");
                jmethodID etCtor = env->GetMethodID(etCls, "<init>", "(Landroid/content/Context;)V");
                jobject etObj = etCtor ? env->NewObject(etCls, etCtor, activityObj) : nullptr;
                env->ExceptionClear();

                if (etObj) {
                    // TYPE_CLASS_TEXT (1) | TYPE_TEXT_FLAG_NO_SUGGESTIONS (0x80000) | TYPE_TEXT_FLAG_MULTI_LINE (0x20000)
                    // (Do NOT set 0x90 VISIBLE_PASSWORD so Android keyboards allow switching to Arabic)
                    jmethodID setInputTypeMid = env->GetMethodID(etCls, "setInputType", "(I)V");
                    if (setInputTypeMid) env->CallVoidMethod(etObj, setInputTypeMid, (jint)0x000A0001);
                    env->ExceptionClear();

                    // IME_FLAG_NO_EXTRACT_UI (0x10000000) | IME_FLAG_NO_FULLSCREEN (0x2000000) | IME_ACTION_DONE (6)
                    jmethodID setImeOptMid = env->GetMethodID(etCls, "setImeOptions", "(I)V");
                    if (setImeOptMid) env->CallVoidMethod(etObj, setImeOptMid, (jint)0x12000006);
                    env->ExceptionClear();

                    jmethodID setBgMid = env->GetMethodID(etCls, "setBackgroundColor", "(I)V");
                    if (setBgMid) env->CallVoidMethod(etObj, setBgMid, (jint)0);
                    jmethodID setTcMid = env->GetMethodID(etCls, "setTextColor", "(I)V");
                    if (setTcMid) env->CallVoidMethod(etObj, setTcMid, (jint)0);
                    jmethodID setCurMid = env->GetMethodID(etCls, "setCursorVisible", "(Z)V");
                    if (setCurMid) env->CallVoidMethod(etObj, setCurMid, (jboolean)JNI_FALSE);
                    jmethodID setFocMid = env->GetMethodID(etCls, "setFocusable", "(Z)V");
                    if (setFocMid) env->CallVoidMethod(etObj, setFocMid, (jboolean)JNI_TRUE);
                    jmethodID setFocTouchMid = env->GetMethodID(etCls, "setFocusableInTouchMode", "(Z)V");
                    if (setFocTouchMid) env->CallVoidMethod(etObj, setFocTouchMid, (jboolean)JNI_TRUE);
                    env->ExceptionClear();

                    jclass lpCls = env->FindClass("android/view/ViewGroup$LayoutParams");
                    jmethodID lpCtor = env->GetMethodID(lpCls, "<init>", "(II)V");
                    jobject lpObj = lpCtor ? env->NewObject(lpCls, lpCtor, (jint)1, (jint)1) : nullptr;
                    env->ExceptionClear();

                    jclass vgCls = env->FindClass("android/view/ViewGroup");
                    jclass viewCls = env->FindClass("android/view/View");
                    jmethodID getChildCountMid = env->GetMethodID(vgCls, "getChildCount", "()I");
                    jmethodID getChildAtMid = env->GetMethodID(vgCls, "getChildAt", "(I)Landroid/view/View;");
                    jmethodID setChildFocMid = env->GetMethodID(viewCls, "setFocusable", "(Z)V");
                    jmethodID setChildFocTouchMid = env->GetMethodID(viewCls, "setFocusableInTouchMode", "(Z)V");
                    // Prevent GLSurfaceView from stealing input focus away from s_pLiveEditText on touch
                    if (getChildCountMid && getChildAtMid && setChildFocMid && setChildFocTouchMid) {
                        jint cCount = env->CallIntMethod(contentView, getChildCountMid);
                        for (jint ci = 0; ci < cCount; ++ci) {
                            jobject ch = env->CallObjectMethod(contentView, getChildAtMid, ci);
                            if (ch) {
                                env->CallVoidMethod(ch, setChildFocMid, (jboolean)JNI_FALSE);
                                env->CallVoidMethod(ch, setChildFocTouchMid, (jboolean)JNI_FALSE);
                                env->DeleteLocalRef(ch);
                            }
                        }
                        env->ExceptionClear();
                    }
                    env->DeleteLocalRef(viewCls);

                    jmethodID addViewMid = env->GetMethodID(vgCls, "addView", "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V");
                    if (addViewMid && lpObj) {
                        env->CallVoidMethod(contentView, addViewMid, etObj, lpObj);
                        if (!env->ExceptionCheck()) {
                            s_pLiveEditText = env->NewGlobalRef(etObj);
                            s_bResetLiveEditTextPending = true;
                        }
                        env->ExceptionClear();
                    }
                    if (lpObj) env->DeleteLocalRef(lpObj);
                    env->DeleteLocalRef(lpCls);
                    env->DeleteLocalRef(vgCls);
                    env->DeleteLocalRef(etObj);
                }
                env->DeleteLocalRef(etCls);
                env->DeleteLocalRef(contentView);
            }
            env->DeleteLocalRef(actCls);
            env->DeleteLocalRef(activityObj);
        }
    }

    if (s_pLiveEditText) {
        jclass etCls = env->FindClass("android/widget/EditText");
        if (s_bResetLiveEditTextPending || !s_bSoftKeyboardVisible) {
            s_bResetLiveEditTextPending = false;
            jstring padStr = env->NewStringUTF("                                ");
            jclass tvCls = env->FindClass("android/widget/TextView");
            jmethodID setTextMid = env->GetMethodID(tvCls, "setText", "(Ljava/lang/CharSequence;)V");
            if (setTextMid) env->CallVoidMethod(s_pLiveEditText, setTextMid, padStr);
            env->ExceptionClear();
            jmethodID setSelMid = env->GetMethodID(etCls, "setSelection", "(I)V");
            if (setSelMid) env->CallVoidMethod(s_pLiveEditText, setSelMid, (jint)32);
            env->ExceptionClear();
            s_u16LastEditText.assign(32, u' ');
            env->DeleteLocalRef(tvCls);
            env->DeleteLocalRef(padStr);
        }
        jclass viewCls = env->FindClass("android/view/View");
        jmethodID reqFocMid = env->GetMethodID(viewCls, "requestFocus", "()Z");
        if (reqFocMid) env->CallBooleanMethod(s_pLiveEditText, reqFocMid);
        env->ExceptionClear();
        jmethodID setSelMid = env->GetMethodID(etCls, "setSelection", "(I)V");
        if (setSelMid) env->CallVoidMethod(s_pLiveEditText, setSelMid, (jint)s_u16LastEditText.size());
        env->ExceptionClear();
        env->DeleteLocalRef(viewCls);
        env->DeleteLocalRef(etCls);
    }
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

    static jmethodID s_getTextMid = nullptr;
    static jmethodID s_toStringMid = nullptr;
    if (!s_getTextMid) {
        jclass tvCls = env->FindClass("android/widget/TextView");
        s_getTextMid = env->GetMethodID(tvCls, "getText", "()Ljava/lang/CharSequence;");
        env->DeleteLocalRef(tvCls);
        jclass objCls = env->FindClass("java/lang/Object");
        s_toStringMid = env->GetMethodID(objCls, "toString", "()Ljava/lang/String;");
        env->DeleteLocalRef(objCls);
    }
    if (!s_getTextMid || !s_toStringMid)
        return false;

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

    if (cleanStr.size() > 32 && cleanStr[0] != u' ' && s_u16LastEditText.size() >= 32 && s_u16LastEditText.substr(0, 32) == std::u16string(32, u' ')) {
        size_t trailingSpaces = 0;
        while (trailingSpaces < cleanStr.size() && cleanStr[cleanStr.size() - 1 - trailingSpaces] == u' ') {
            ++trailingSpaces;
        }
        if (trailingSpaces >= 32) {
            std::u16string typedPart = cleanStr.substr(0, cleanStr.size() - 32);
            cleanStr = std::u16string(32, u' ') + typedPart;
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
    if (cleanStr.size() < 8 || cleanStr.size() > 160) {
        s_bResetLiveEditTextPending = true;
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
            bool handledDirectly = false;
            if (s_pLiveEditText) {
                if (show) {
                    jmethodID showMid = env->GetMethodID(immCls, "showSoftInput", "(Landroid/view/View;I)Z");
                    if (showMid) {
                        handledDirectly = (env->CallBooleanMethod(immObj, showMid, s_pLiveEditText, (jint)0) == JNI_TRUE);
                        env->ExceptionClear();
                    }
                } else {
                    jclass viewCls = env->FindClass("android/view/View");
                    jmethodID tokMid = env->GetMethodID(viewCls, "getWindowToken", "()Landroid/os/IBinder;");
                    jobject token = tokMid ? env->CallObjectMethod(s_pLiveEditText, tokMid) : nullptr;
                    env->ExceptionClear();
                    if (token) {
                        jmethodID hideMid = env->GetMethodID(immCls, "hideSoftInputFromWindow", "(Landroid/os/IBinder;I)Z");
                        if (hideMid) {
                            env->CallBooleanMethod(immObj, hideMid, token, (jint)0);
                            handledDirectly = true;
                        }
                        env->ExceptionClear();
                        env->DeleteLocalRef(token);
                    }
                    env->DeleteLocalRef(viewCls);
                }
            }
            if (!handledDirectly) {
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
        bool wasDraggingWnd = slot.isDraggingWindow;
        UI::CWindow* pDraggedWnd = slot.pHoldDragWnd;
        slot.active = false;
        slot.role = TOUCH_ROLE_NONE;
        slot.isDraggingCamera = false;
        slot.isDraggingWindow = false;
        slot.holdDragCancelled = false;
        slot.pHoldDragWnd = nullptr;
        UpdateUiTouchDownFlag();
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
        MapTouchThroughScaledWindows(rx, ry, &mx, &my);
        // If ACTION_DOWN snapped onto a button and the finger didn't drag far, release squarely on the snapped button!
        int totalMoveSq = (rx - slot.startX) * (rx - slot.startX) + (ry - slot.startY) * (ry - slot.startY);
        if (s_bLastTouchSnappedToButton && totalMoveSq <= 324) {
            mx = s_iLastTouchDownX;
            my = s_iLastTouchDownY;
        }
        g_AndroidMouseX = mx;
        g_AndroidMouseY = my;
        UI::CWindowManager::Instance().RunMouseMove(mx, my);
        UI::CWindowManager::Instance().RunMouseLeftButtonUp(mx, my);
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

static PyObject* mobileInput_IsRectTouchHeld(PyObject* self, PyObject* args) {
    int rx = 0, ry = 0, rw = 0, rh = 0;
    if (!PyArg_ParseTuple(args, "iiii", &rx, &ry, &rw, &rh))
        return Py_BuildValue("i", 0);
    for (int i = 0; i < 4; ++i) {
        if (!s_pointers[i].active || s_pointers[i].role == TOUCH_ROLE_JOYSTICK)
            continue;
        int sx = s_pointers[i].startX, sy = s_pointers[i].startY;
        int cx = s_pointers[i].curX,   cy = s_pointers[i].curY;
        if ((sx >= rx && sx < (rx + rw) && sy >= ry && sy < (ry + rh)) ||
            (cx >= rx && cx < (rx + rw) && cy >= ry && cy < (ry + rh)))
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
    if (show != 0) s_bExplicitKeyboardRequest = true;
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

static PyObject* mobileInput_SetWindowScale(PyObject* /*self*/, PyObject* args) {
    PyObject* pyHandle = nullptr;
    float scale = 1.0f;
    if (!PyArg_ParseTuple(args, "Of", &pyHandle, &scale))
        return Py_BuildValue("i", 0);
    UI::CWindow* pWnd = (UI::CWindow*)PyLong_AsVoidPtr(pyHandle);
    if (PyErr_Occurred()) { PyErr_Clear(); pWnd = nullptr; }
    if (pWnd) {
        uintptr_t key = (uintptr_t)pWnd;
        if (fabsf(scale - 1.0f) <= 0.01f || !pWnd->IsShow()) {
            s_scaledWindows.erase(key);
        } else {
            long wx = 0, wy = 0;
            pWnd->GetPosition(&wx, &wy);
            SScaledWindowRect& entry = s_scaledWindows[key];
            entry.scale = scale;
            entry.x = (int)wx;
            entry.y = (int)wy;
            entry.w = pWnd->GetWidth();
            entry.h = pWnd->GetHeight();
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

                // 1. Virtual Joystick (Bottom-Left)
                if (bInGameWorld && !s_bJoyActive && (jdx * jdx + jdy * jdy <= joyCatchRadius * joyCatchRadius)) {
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
                    slot.pHoldDragWnd = s_bLastTouchSnappedToButton ? nullptr : FindHoldDraggableWindow(mx, my);
                    UpdateUiTouchDownFlag();
                    g_AndroidMouseX = mx;
                    g_AndroidMouseY = my;

                    // Check if the touched UI widget belongs to the Chat system; if not, force-close open ChatWindow!
                    UI::CWindow* pTouched = UI::CWindowManager::Instance().GetPointWindow();
                    bool isChatWidget = false;
                    for (UI::CWindow* pWalk = pTouched; pWalk != nullptr; pWalk = pWalk->GetParent()) {
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
                    UI::CWindowManager::Instance().RunMouseMove(mx, my);
                    UI::CWindowManager::Instance().RunMouseLeftButtonDown(mx, my);
                }
                // 3. 3D World (Orbit / Pinch-to-Zoom / Fast-Tap)
                else {
                    s_iLastTouchDownX = ev.x;
                    s_iLastTouchDownY = ev.y;
                    s_bLastTouchSnappedToButton = false;
                    slot.role = TOUCH_ROLE_WORLD;
                    s_fCamVelRoll *= 0.25f;
                    s_fCamVelPitch *= 0.25f;
                    // Auto-close open ChatWindow when tapping on the 3D world
                    PyRun_SimpleString(
                   "try:\n"
                   "    import constInfo\n"
                   "    if hasattr(constInfo, 'CheckAutoCloseMobileChat'): constInfo.CheckAutoCloseMobileChat(-1, -1)\n"
                   "    if hasattr(constInfo, 'DismissMobileTooltipsOnTap'): constInfo.DismissMobileTooltipsOnTap(-5000, -5000)\n"
                   "except:\n"
                   "    pass\n"
               );
               if (!s_bUiTouchDown) {
                   UI::CWindowManager::Instance().RunMouseMove(-5000, -5000);
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
                    DWORD heldMs = ELTimer_GetMSec() - slot.downTimeMs;
                    int totalDx = ev.x - slot.startX;
                    int totalDy = ev.y - slot.startY;
                    int distSq = totalDx * totalDx + totalDy * totalDy;

                    if (!slot.isDraggingWindow && !slot.holdDragCancelled) {
                        if (heldMs >= 120 && slot.pHoldDragWnd != nullptr && distSq > 16) {
                            // Easy & responsive hold-drag after 120ms!
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
                        MapTouchThroughScaledWindows(ev.x, ev.y, &mx, &my);
                        if (s_bLastTouchSnappedToButton && distSq <= 324) {
                            mx = s_iLastTouchDownX;
                            my = s_iLastTouchDownY;
                        }
                        g_AndroidMouseX = mx;
                        g_AndroidMouseY = my;
                        UI::CWindowManager::Instance().RunMouseMove(mx, my);
                        SanitizeButtonChildPickFlags(UI::CWindowManager::Instance().GetPointWindow());
                    }
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