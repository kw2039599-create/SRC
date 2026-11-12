#include "StdAfx.h"
#include "GrpOpenGL.h"

#if defined(ENABLE_OPENGL) && defined(ANDROID)

#if defined(__ANDROID__) && !defined(__APPLE__)
#include <android/log.h>
#define LOG_TAG "Metin2Mobile"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
#include <stdio.h>
#define LOGI(...) do { fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); } while(0)
#define LOGE(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while(0)
#endif
#ifdef __ANDROID__
extern "C" int g_AndroidPhysWidth;
extern "C" int g_AndroidPhysHeight;

#define ENABLE_MOBILE_PERF_DEBUGGER // <-- Comment out to disable the profiler

#ifdef ENABLE_MOBILE_PERF_DEBUGGER
#include <time.h>

enum EMobileRenderBucket {
    RBUCK_SHADOW_DEPTH = 0,
    RBUCK_REFL_FBO     = 1,
    RBUCK_TERRAIN      = 2,
    RBUCK_MAP_OBJECTS  = 3,
    RBUCK_WATER        = 4,
    RBUCK_ACTORS_GR2   = 5,
    RBUCK_EFFECTS_3D   = 6,
    RBUCK_UI_TEXT      = 7,
    RBUCK_COUNT        = 8
};

struct SMobileGLFrameStats {
    uint32_t drawCalls[RBUCK_COUNT];
    uint32_t tris[RBUCK_COUNT];
    uint64_t cpuTimeNs[RBUCK_COUNT];
    uint32_t vboUploads;
    uint32_t vboUploadBytes;
    uint32_t texBinds;
    uint32_t progSwitches;
    uint32_t fboSwitches;
};

extern "C" SMobileGLFrameStats g_mobileGLStats = {};

static inline uint64_t GetPerfTimeNs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static inline int ClassifyDrawBucket(const IDirect3DDevice8* dev, bool isUPDraw) {
    const int sm = dev->m_ActivePixelShaderType;
    if (dev->m_pCurrentRenderTargetTex != NULL)
        return (sm == 1) ? RBUCK_SHADOW_DEPTH : RBUCK_REFL_FBO;
    if (sm == 1) return RBUCK_SHADOW_DEPTH;
    if (sm == 2 || sm == 3) return RBUCK_TERRAIN;
    if (sm == 4 || sm == 7) return RBUCK_MAP_OBJECTS;
    if (sm == 5) return RBUCK_WATER;
    if (!isUPDraw) return RBUCK_ACTORS_GR2;
    if (dev->m_RenderStates[D3DRS_ZENABLE] == 0 ||
        ((dev->m_dwFVF & 0x00E) == D3DFVF_XYZRHW) ||
        fabsf(dev->m_matProj._34) < 1e-4f) {
        return RBUCK_UI_TEXT;
    }
    return RBUCK_EFFECTS_3D;
}
#endif
#endif

namespace {
    // ========================================================================
    // 1. Lightweight Mode 0 Shaders (Characters, NPCs, Monsters, Trees, Effects, UI)
    // ========================================================================
    const char* vShaderMode0 =
        "#version 300 es\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "layout(location = 0) in vec3 aPos;\n"
        "layout(location = 1) in vec2 aTexCoord0;\n"
        "layout(location = 2) in vec4 aColor;\n"
        "layout(location = 3) in vec2 aTexCoord1;\n"
        "layout(location = 4) in vec3 aNormal;\n"
        "uniform mat4 uProjection;\n"
        "uniform mat4 uModelView;\n"
        "uniform mat4 uWorld;\n"
        "uniform mat4 uTexMatrix0;\n"
        "uniform mat4 uTexMatrix1;\n"
        "uniform int uUseTexMat0;\n"
        "uniform int uUseTexMat1;\n"
        "uniform int uTexGenMode0;\n"
        "uniform int uTexGenMode1;\n"
        "uniform int uHasNormal;\n"
        "uniform int uHasVertexColor;\n"
        "uniform int uLightingEnable;\n"
        "uniform int uRenderToFBO;\n"
        "uniform vec3 uLightDir;\n"
        "uniform vec3 uLightDiffuse;\n"
        "uniform vec3 uLightAmbient;\n"
        "uniform vec4 uMatDiffuse;\n"
        "uniform vec3 uMatAmbient;\n"
        "uniform vec3 uMatEmissive;\n"
        "uniform vec3 uGlobalAmbient;\n"
        "out vec2 vTexCoord0;\n"
        "out vec2 vTexCoord1;\n"
        "out vec4 vColor;\n"
        "out float vEyeDist;\n"
        "out vec3 vWorldPos;\n"
        "void main() {\n"
        "  vec4 eyePos = uModelView * vec4(aPos, 1.0);\n"
        "  vec4 clipPos = uProjection * eyePos;\n"
        "  if (uRenderToFBO != 0) {\n"
        "    clipPos.z = clipPos.z * 2.0 - clipPos.w;\n"
        "    clipPos.y = -clipPos.y;\n"
        "  }\n"
        "  gl_Position = clipPos;\n"
        "  vWorldPos = (uWorld * vec4(aPos, 1.0)).xyz;\n"
        "  vEyeDist = length(eyePos.xyz);\n"
        "  vec3 eyeNormal = (uHasNormal != 0) ? normalize(mat3(uModelView) * aNormal) : vec3(0.0, 0.0, 1.0);\n"
        "  vec4 tc0 = vec4(aTexCoord0, 0.0, 1.0);\n"
        "  if (uTexGenMode0 == 1) tc0 = vec4(eyeNormal, 1.0);\n"
        "  else if (uTexGenMode0 == 2) tc0 = eyePos;\n"
        "  else if (uTexGenMode0 == 3) {\n"
        "    vec3 r0 = reflect(normalize(eyePos.xyz), eyeNormal);\n"
        "    tc0 = vec4(0.5 * r0.x + 0.5, -0.5 * r0.y + 0.5, r0.z, 1.0);\n"
        "  }\n"
        "  vec4 tc1 = vec4(aTexCoord1, 0.0, 1.0);\n"
        "  if (uTexGenMode1 == 1) tc1 = vec4(eyeNormal, 1.0);\n"
        "  else if (uTexGenMode1 == 2) tc1 = eyePos;\n"
        "  else if (uTexGenMode1 == 3) {\n"
        "    vec3 r1 = reflect(normalize(eyePos.xyz), eyeNormal);\n"
        "    tc1 = vec4(0.5 * r1.x + 0.5, -0.5 * r1.y + 0.5, r1.z, 1.0);\n"
        "  }\n"
        "  if (uUseTexMat0 != 0) tc0 = uTexMatrix0 * tc0;\n"
        "  if (uUseTexMat1 != 0) tc1 = uTexMatrix1 * tc1;\n"
        "  vTexCoord0 = tc0.xy;\n"
        "  vTexCoord1 = tc1.xy;\n"
        "  vec4 baseCol = aColor.bgra;\n"
        "  if (uLightingEnable != 0 && uHasNormal != 0) {\n"
        "    vec3 wNorm = normalize(mat3(uWorld) * aNormal);\n"
        "    float ndotl = max(dot(wNorm, -uLightDir), 0.0);\n"
        "    vec3 diffSrc = (uHasVertexColor != 0) ? baseCol.rgb : uMatDiffuse.rgb;\n"
        "    vec3 ambSrc  = (uHasVertexColor != 0) ? baseCol.rgb : uMatAmbient.rgb;\n"
        "    vec3 lit = ambSrc * (uGlobalAmbient + uLightAmbient) + ndotl * uLightDiffuse * diffSrc + uMatEmissive;\n"
        "    float alphaOut = (uHasVertexColor != 0) ? baseCol.a : uMatDiffuse.a;\n"
        "    vColor = vec4(clamp(lit, 0.0, 1.0), alphaOut);\n"
        "  } else {\n"
        "    vColor = baseCol;\n"
        "  }\n"
        "}\n";

    #define MODE0_COMBINE_HELPERS \
        "vec3 EvalColorArg(int arg, vec4 tex, vec4 cur, vec4 diff, vec4 tf) {\n" \
        "  int s = arg & 15;\n" \
        "  vec4 src = tex;\n" \
        "  if (s == 0) src = diff;\n" \
        "  else if (s == 1) src = cur;\n" \
        "  else if (s == 2) src = tex;\n" \
        "  else if (s == 3) src = tf;\n" \
        "  vec3 r = ((arg & 32) != 0) ? vec3(src.a) : src.rgb;\n" \
        "  if ((arg & 16) != 0) r = vec3(1.0) - r;\n" \
        "  return r;\n" \
        "}\n" \
        "float EvalAlphaArg(int arg, float texA, float curA, float diffA, float tfA) {\n" \
        "  int s = arg & 15;\n" \
        "  float r = texA;\n" \
        "  if (s == 0) r = diffA;\n" \
        "  else if (s == 1) r = curA;\n" \
        "  else if (s == 2) r = texA;\n" \
        "  else if (s == 3) r = tfA;\n" \
        "  if ((arg & 16) != 0) r = 1.0 - r;\n" \
        "  return r;\n" \
        "}\n" \
        "vec3 CombineColor(int op, vec3 a1, vec3 a2, float aa1, float texA, float diffA, float tfA, float curA) {\n" \
        "  if (op == 2) return a1;\n" \
        "  if (op == 3) return a2;\n" \
        "  if (op == 4) return a1 * a2;\n" \
        "  if (op == 5) return clamp(a1 * a2 * 2.0, 0.0, 1.0);\n" \
        "  if (op == 6) return clamp(a1 * a2 * 4.0, 0.0, 1.0);\n" \
        "  if (op == 7) return clamp(a1 + a2, 0.0, 1.0);\n" \
        "  if (op == 8) return clamp(a1 + a2 - vec3(0.5), 0.0, 1.0);\n" \
        "  if (op == 9) return clamp((a1 + a2 - vec3(0.5)) * 2.0, 0.0, 1.0);\n" \
        "  if (op == 10) return clamp(a1 - a2, 0.0, 1.0);\n" \
        "  if (op == 11) return clamp(a1 + a2 - a1 * a2, 0.0, 1.0);\n" \
        "  if (op == 12) return mix(a2, a1, diffA);\n" \
        "  if (op == 13) return mix(a2, a1, texA);\n" \
        "  if (op == 14) return mix(a2, a1, tfA);\n" \
        "  if (op == 15) return clamp(a1 + a2 * (1.0 - texA), 0.0, 1.0);\n" \
        "  if (op == 16) return mix(a2, a1, curA);\n" \
        "  if (op == 18) return clamp(a1 + aa1 * a2, 0.0, 1.0);\n" \
        "  if (op == 19) return clamp(a1 * a2 + vec3(aa1), 0.0, 1.0);\n" \
        "  if (op == 20) return clamp(a1 + (1.0 - aa1) * a2, 0.0, 1.0);\n" \
        "  if (op == 21) return clamp((vec3(1.0) - a1) * a2 + vec3(aa1), 0.0, 1.0);\n" \
        "  return a1 * a2;\n" \
        "}\n" \
        "float CombineAlpha(int op, float a1, float a2, float curA) {\n" \
        "  if (op == 1) return curA;\n" \
        "  if (op == 2) return a1;\n" \
        "  if (op == 3) return a2;\n" \
        "  if (op == 4) return a1 * a2;\n" \
        "  if (op == 5) return clamp(a1 * a2 * 2.0, 0.0, 1.0);\n" \
        "  if (op == 6) return clamp(a1 * a2 * 4.0, 0.0, 1.0);\n" \
        "  if (op == 7) return clamp(a1 + a2, 0.0, 1.0);\n" \
        "  if (op == 8) return clamp(a1 + a2 - 0.5, 0.0, 1.0);\n" \
        "  if (op == 9) return clamp((a1 + a2 - 0.5) * 2.0, 0.0, 1.0);\n" \
        "  if (op == 10) return clamp(a1 - a2, 0.0, 1.0);\n" \
        "  if (op == 11) return clamp(a1 + a2 - a1 * a2, 0.0, 1.0);\n" \
        "  return a1 * a2;\n" \
        "}\n"

    // Mode 0 Opaque/Blended: ZERO discard statements -> 100% Hardware Early-Z & HSR!
    const char* fShaderMode0Opaque =
        "#version 300 es\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "in vec2 vTexCoord0;\n"
        "in vec2 vTexCoord1;\n"
        "in vec4 vColor;\n"
        "in float vEyeDist;\n"
        "in vec3 vWorldPos;\n"
        "out vec4 FragColor;\n"
        "uniform sampler2D uTexture0;\n"
        "uniform sampler2D uTexture1;\n"
        "uniform int uHasTexture0;\n"
        "uniform int uHasTexture1;\n"
        "uniform int uColorOp0;\n"
        "uniform int uColorArg1_0;\n"
        "uniform int uColorArg2_0;\n"
        "uniform int uAlphaOp0;\n"
        "uniform int uAlphaArg1_0;\n"
        "uniform int uAlphaArg2_0;\n"
        "uniform int uColorOp1;\n"
        "uniform int uColorArg1_1;\n"
        "uniform int uColorArg2_1;\n"
        "uniform int uAlphaOp1;\n"
        "uniform int uAlphaArg1_1;\n"
        "uniform int uAlphaArg2_1;\n"
        "uniform vec4 uTexFactor;\n"
        "uniform int uFogEnable;\n"
        "uniform vec4 uFogColor;\n"
        "uniform vec2 uFogRange;\n"
        "uniform vec2 uPsSpecTrans;\n"
        MODE0_COMBINE_HELPERS
        "void main() {\n"
        "  vec4 c = vColor;\n"
        "  if (uColorOp0 != 1) {\n"
        "    vec4 t0 = (uHasTexture0 != 0) ? texture(uTexture0, vTexCoord0) : vec4(1.0);\n"
        "    vec3 ca1 = EvalColorArg(uColorArg1_0, t0, vColor, vColor, uTexFactor);\n"
        "    vec3 ca2 = EvalColorArg(uColorArg2_0, t0, vColor, vColor, uTexFactor);\n"
        "    float aa1 = EvalAlphaArg(uAlphaArg1_0, t0.a, vColor.a, vColor.a, uTexFactor.a);\n"
        "    float aa2 = EvalAlphaArg(uAlphaArg2_0, t0.a, vColor.a, vColor.a, uTexFactor.a);\n"
        "    c.rgb = CombineColor(uColorOp0, ca1, ca2, aa1, t0.a, vColor.a, uTexFactor.a, vColor.a);\n"
        "    c.a = CombineAlpha(uAlphaOp0, aa1, aa2, vColor.a);\n"
        "  }\n"
        "  if (uColorOp1 != 1 && (uHasTexture1 != 0 || ((uColorArg1_1 & 15) != 2 && (uColorArg2_1 & 15) != 2))) {\n"
        "    vec2 uv1 = (uColorOp1 == 18) ? fract(vTexCoord1 + uPsSpecTrans) : vTexCoord1;\n"
        "    vec4 t1 = (uHasTexture1 != 0) ? texture(uTexture1, uv1) : vec4(1.0);\n"
        "    vec3 ca1 = EvalColorArg(uColorArg1_1, t1, c, vColor, uTexFactor);\n"
        "    vec3 ca2 = EvalColorArg(uColorArg2_1, t1, c, vColor, uTexFactor);\n"
        "    float aa1 = EvalAlphaArg(uAlphaArg1_1, t1.a, c.a, vColor.a, uTexFactor.a);\n"
        "    float aa2 = EvalAlphaArg(uAlphaArg2_1, t1.a, c.a, vColor.a, uTexFactor.a);\n"
        "    c.rgb = CombineColor(uColorOp1, ca1, ca2, aa1, t1.a, vColor.a, uTexFactor.a, c.a);\n"
        "    c.a = CombineAlpha(uAlphaOp1, aa1, aa2, c.a);\n"
        "  }\n"
        "  if (uFogEnable != 0 && uFogRange.y > uFogRange.x) {\n"
        "    float fogFactor = clamp((vEyeDist - uFogRange.x) / (uFogRange.y - uFogRange.x), 0.0, 1.0);\n"
        "    c.rgb = mix(c.rgb, uFogColor.rgb, fogFactor);\n"
        "  }\n"
        "  FragColor = c;\n"
        "}\n";

    // Mode 0 AlphaTest / ClipPlane
    const char* fShaderMode0Discard =
        "#version 300 es\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "in vec2 vTexCoord0;\n"
        "in vec2 vTexCoord1;\n"
        "in vec4 vColor;\n"
        "in float vEyeDist;\n"
        "in vec3 vWorldPos;\n"
        "out vec4 FragColor;\n"
        "uniform sampler2D uTexture0;\n"
        "uniform sampler2D uTexture1;\n"
        "uniform int uHasTexture0;\n"
        "uniform int uHasTexture1;\n"
        "uniform int uColorOp0;\n"
        "uniform int uColorArg1_0;\n"
        "uniform int uColorArg2_0;\n"
        "uniform int uAlphaOp0;\n"
        "uniform int uAlphaArg1_0;\n"
        "uniform int uAlphaArg2_0;\n"
        "uniform int uColorOp1;\n"
        "uniform int uColorArg1_1;\n"
        "uniform int uColorArg2_1;\n"
        "uniform int uAlphaOp1;\n"
        "uniform int uAlphaArg1_1;\n"
        "uniform int uAlphaArg2_1;\n"
        "uniform vec4 uTexFactor;\n"
        "uniform int uAlphaTest;\n"
        "uniform float uAlphaRef;\n"
        "uniform int uFogEnable;\n"
        "uniform vec4 uFogColor;\n"
        "uniform vec2 uFogRange;\n"
        "uniform int uClipPlaneEnable;\n"
        "uniform vec4 uClipPlane0;\n"
        "uniform vec2 uPsSpecTrans;\n"
        MODE0_COMBINE_HELPERS
        "void main() {\n"
        "  if (uClipPlaneEnable != 0 && dot(vec4(vWorldPos, 1.0), uClipPlane0) < 0.0) discard;\n"
        "  vec4 c = vColor;\n"
        "  if (uColorOp0 != 1) {\n"
        "    vec4 t0 = (uHasTexture0 != 0) ? texture(uTexture0, vTexCoord0) : vec4(1.0);\n"
        "    vec3 ca1 = EvalColorArg(uColorArg1_0, t0, vColor, vColor, uTexFactor);\n"
        "    vec3 ca2 = EvalColorArg(uColorArg2_0, t0, vColor, vColor, uTexFactor);\n"
        "    float aa1 = EvalAlphaArg(uAlphaArg1_0, t0.a, vColor.a, vColor.a, uTexFactor.a);\n"
        "    float aa2 = EvalAlphaArg(uAlphaArg2_0, t0.a, vColor.a, vColor.a, uTexFactor.a);\n"
        "    c.rgb = CombineColor(uColorOp0, ca1, ca2, aa1, t0.a, vColor.a, uTexFactor.a, vColor.a);\n"
        "    c.a = CombineAlpha(uAlphaOp0, aa1, aa2, vColor.a);\n"
        "  }\n"
        "  if (uColorOp1 != 1 && (uHasTexture1 != 0 || ((uColorArg1_1 & 15) != 2 && (uColorArg2_1 & 15) != 2))) {\n"
        "    vec2 uv1 = (uColorOp1 == 18) ? fract(vTexCoord1 + uPsSpecTrans) : vTexCoord1;\n"
        "    vec4 t1 = (uHasTexture1 != 0) ? texture(uTexture1, uv1) : vec4(1.0);\n"
        "    vec3 ca1 = EvalColorArg(uColorArg1_1, t1, c, vColor, uTexFactor);\n"
        "    vec3 ca2 = EvalColorArg(uColorArg2_1, t1, c, vColor, uTexFactor);\n"
        "    float aa1 = EvalAlphaArg(uAlphaArg1_1, t1.a, c.a, vColor.a, uTexFactor.a);\n"
        "    float aa2 = EvalAlphaArg(uAlphaArg2_1, t1.a, c.a, vColor.a, uTexFactor.a);\n"
        "    c.rgb = CombineColor(uColorOp1, ca1, ca2, aa1, t1.a, vColor.a, uTexFactor.a, c.a);\n"
        "    c.a = CombineAlpha(uAlphaOp1, aa1, aa2, c.a);\n"
        "  }\n"
        "  if (uAlphaTest != 0 && c.a <= uAlphaRef) discard;\n"
        "  if (uFogEnable != 0 && uFogRange.y > uFogRange.x) {\n"
        "    float fogFactor = clamp((vEyeDist - uFogRange.x) / (uFogRange.y - uFogRange.x), 0.0, 1.0);\n"
        "    c.rgb = mix(c.rgb, uFogColor.rgb, fogFactor);\n"
        "  }\n"
        "  FragColor = c;\n"
        "}\n";

    // ========================================================================
    // 2. Ultra-Fast Mode 1 Shaders (Shadow Map Depth Pass)
    // ========================================================================
    const char* vShaderMode1 =
        "#version 300 es\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "layout(location = 0) in vec3 aPos;\n"
        "layout(location = 1) in vec2 aTexCoord0;\n"
        "uniform mat4 uProjection;\n"
        "uniform mat4 uModelView;\n"
        "uniform int uRenderToFBO;\n"
        "out vec2 vTexCoord0;\n"
        "out float vShadowDepth;\n"
        "void main() {\n"
        "  vec4 clipPos = uProjection * (uModelView * vec4(aPos, 1.0));\n"
        "  vShadowDepth = clipPos.z;\n"
        "  if (uRenderToFBO != 0) {\n"
        "    clipPos.z = clipPos.z * 2.0 - clipPos.w;\n"
        "    clipPos.y = -clipPos.y;\n"
        "  }\n"
        "  gl_Position = clipPos;\n"
        "  vTexCoord0 = aTexCoord0;\n"
        "}\n";

    const char* fShaderMode1Opaque =
        "#version 300 es\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "in vec2 vTexCoord0;\n"
        "in float vShadowDepth;\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "  float d = clamp(vShadowDepth, 0.0, 1.0);\n"
        "  vec3 enc = fract(vec3(1.0, 255.0, 65025.0) * d);\n"
        "  enc.xy -= enc.yz / 255.0;\n"
        "  FragColor = vec4(enc, 1.0);\n"
        "}\n";

    const char* fShaderMode1Discard =
        "#version 300 es\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "in vec2 vTexCoord0;\n"
        "in float vShadowDepth;\n"
        "out vec4 FragColor;\n"
        "uniform sampler2D uTexture0;\n"
        "uniform float uAlphaRef;\n"
        "void main() {\n"
        "  if (texture(uTexture0, vTexCoord0).a <= uAlphaRef) discard;\n"
        "  float d = clamp(vShadowDepth, 0.0, 1.0);\n"
        "  vec3 enc = fract(vec3(1.0, 255.0, 65025.0) * d);\n"
        "  enc.xy -= enc.yz / 255.0;\n"
        "  FragColor = vec4(enc, 1.0);\n"
        "}\n";

    // ========================================================================
    // 3. Advanced Shader Modes 2..7 (Terrain, Cascades, Shadowed Meshes, Water)
    // ========================================================================
    const char* vShaderUber =
        "#version 300 es\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "layout(location = 0) in vec3 aPos;\n"
        "layout(location = 1) in vec2 aTexCoord0;\n"
        "layout(location = 2) in vec4 aColor;\n"
        "layout(location = 3) in vec2 aTexCoord1;\n"
        "layout(location = 4) in vec3 aNormal;\n"
        "uniform mat4 uProjection;\n"
        "uniform mat4 uModelView;\n"
        "uniform mat4 uWorld;\n"
        "uniform mat4 uTexMatrix0;\n"
        "uniform mat4 uTexMatrix1;\n"
        "uniform mat4 uLightViewProj1;\n"
        "uniform mat4 uLightViewProj2;\n"
        "uniform mat4 uTerrainColorMat;\n"
        "uniform mat4 uTerrainAlphaMat;\n"
        "uniform int uUseTexMat0;\n"
        "uniform int uUseTexMat1;\n"
        "uniform int uTexGenMode0;\n"
        "uniform int uTexGenMode1;\n"
        "uniform int uHasNormal;\n"
        "uniform int uHasVertexColor;\n"
        "uniform int uShaderMode;\n"
        "uniform int uRenderToFBO;\n"
        "uniform vec4 uMatDiffuse;\n"
        "out vec2 vTexCoord0;\n"
        "out vec2 vTexCoord1;\n"
        "out vec4 vColor;\n"
        "out float vEyeDist;\n"
        "out vec3 vWorldPos;\n"
        "out vec3 vWorldNormal;\n"
        "out vec3 vLightPos1;\n"
        "out vec3 vLightPos2;\n"
        "out vec4 vScreenPos;\n"
        "void main() {\n"
        "  vec4 eyePos = uModelView * vec4(aPos, 1.0);\n"
        "  vec4 clipPos = uProjection * eyePos;\n"
        "  if (uRenderToFBO != 0) {\n"
        "    clipPos.z = clipPos.z * 2.0 - clipPos.w;\n"
        "    clipPos.y = -clipPos.y;\n"
        "  }\n"
        "  gl_Position = clipPos;\n"
        "  vec4 worldPos4 = uWorld * vec4(aPos, 1.0);\n"
        "  vWorldPos = worldPos4.xyz;\n"
        "  vWorldNormal = (uHasNormal != 0) ? normalize(mat3(uWorld) * aNormal) : vec3(0.0, 0.0, 1.0);\n"
        "  vEyeDist = length(eyePos.xyz);\n"
        "  vScreenPos = clipPos;\n"
        "  vLightPos1 = (uLightViewProj1 * worldPos4).xyz;\n"
        "  vLightPos2 = (uLightViewProj2 * worldPos4).xyz;\n"
        "  if (uShaderMode == 2) {\n"
        "    vTexCoord0 = (uTerrainColorMat * worldPos4).xy;\n"
        "    vTexCoord1 = (uTerrainAlphaMat * vec4(aPos, 1.0)).xy;\n"
        "    vColor = vec4(1.0);\n"
        "    return;\n"
        "  }\n"
        "  if (uShaderMode == 5) {\n"
        "    vTexCoord0 = worldPos4.xy * 0.001;\n"
        "    vTexCoord1 = aTexCoord0;\n"
        "    vColor = aColor.bgra;\n"
        "    return;\n"
        "  }\n"
        "  vec3 eyeNormal = (uHasNormal != 0) ? normalize(mat3(uModelView) * aNormal) : vec3(0.0, 0.0, 1.0);\n"
        "  vec4 tc0 = vec4(aTexCoord0, 0.0, 1.0);\n"
        "  if (uTexGenMode0 == 1) tc0 = vec4(eyeNormal, 1.0);\n"
        "  else if (uTexGenMode0 == 2) tc0 = eyePos;\n"
        "  else if (uTexGenMode0 == 3) {\n"
        "    vec3 r0 = reflect(normalize(eyePos.xyz), eyeNormal);\n"
        "    tc0 = vec4(0.5 * r0.x + 0.5, -0.5 * r0.y + 0.5, r0.z, 1.0);\n"
        "  }\n"
        "  vec4 tc1 = vec4(aTexCoord1, 0.0, 1.0);\n"
        "  if (uTexGenMode1 == 1) tc1 = vec4(eyeNormal, 1.0);\n"
        "  else if (uTexGenMode1 == 2) tc1 = eyePos;\n"
        "  else if (uTexGenMode1 == 3) {\n"
        "    vec3 r1 = reflect(normalize(eyePos.xyz), eyeNormal);\n"
        "    tc1 = vec4(0.5 * r1.x + 0.5, -0.5 * r1.y + 0.5, r1.z, 1.0);\n"
        "  }\n"
        "  if (uUseTexMat0 != 0) tc0 = uTexMatrix0 * tc0;\n"
        "  if (uUseTexMat1 != 0) tc1 = uTexMatrix1 * tc1;\n"
        "  vTexCoord0 = tc0.xy;\n"
        "  vTexCoord1 = tc1.xy;\n"
        "  vec4 baseCol = aColor.bgra;\n"
        "  vColor = vec4(1.0, 1.0, 1.0, (uHasVertexColor != 0) ? baseCol.a : uMatDiffuse.a);\n"
        "}\n";

    #define UBER_FRAGMENT_COMMON \
        "#version 300 es\n" \
        "precision highp float;\n" \
        "precision highp int;\n" \
        "in vec2 vTexCoord0;\n" \
        "in vec2 vTexCoord1;\n" \
        "in vec4 vColor;\n" \
        "in float vEyeDist;\n" \
        "in vec3 vWorldPos;\n" \
        "in vec3 vWorldNormal;\n" \
        "in vec3 vLightPos1;\n" \
        "in vec3 vLightPos2;\n" \
        "in vec4 vScreenPos;\n" \
        "out vec4 FragColor;\n" \
        "uniform sampler2D uTexture0;\n" \
        "uniform sampler2D uTexture1;\n" \
        "uniform sampler2D uTexture2;\n" \
        "uniform sampler2D uTexture3;\n" \
        "uniform sampler2D uTexture4;\n" \
        "uniform sampler2D uTexture5;\n" \
        "uniform sampler2D uTexture6;\n" \
        "uniform sampler2D uTexture7;\n" \
        "uniform int uHasTexture0;\n" \
        "uniform int uHasTexture1;\n" \
        "uniform int uHasTexture2;\n" \
        "uniform int uHasTexture3;\n" \
        "uniform int uHasTexture4;\n" \
        "uniform int uHasTexture5;\n" \
        "uniform int uHasTexture6;\n" \
        "uniform int uHasTexture7;\n" \
        "uniform int uColorOp1;\n" \
        "uniform vec4 uTexFactor;\n" \
        "uniform int uAlphaTest;\n" \
        "uniform float uAlphaRef;\n" \
        "uniform vec4 uFogColor;\n" \
        "uniform vec2 uFogRange;\n" \
        "uniform int uShaderMode;\n" \
        "uniform int uRenderToFBO;\n" \
        "uniform int uClipPlaneEnable;\n" \
        "uniform vec4 uClipPlane0;\n" \
        "uniform vec3 uPsMatDiffuse;\n" \
        "uniform vec3 uPsSunDir;\n" \
        "uniform vec3 uPsLightDiffuse;\n" \
        "uniform vec3 uPsAmbient;\n" \
        "uniform vec3 uPsCameraPos;\n" \
        "uniform vec3 uPsMatSpecular;\n" \
        "uniform float uPsShadowOpacity;\n" \
        "uniform float uPsSpecularEnabled;\n" \
        "uniform float uPsShininess;\n" \
        "uniform vec3 uPsSpecularLight;\n" \
        "uniform float uPsAlphaOverride;\n" \
        "uniform float uPsAlphaVal;\n" \
        "uniform float uPsTime;\n" \
        "uniform float uPsUVScale;\n" \
        "uniform float uPsPuddleThreshold;\n" \
        "uniform vec3 uPsCharPos;\n" \
        "uniform vec3 uPsPrevCharPos;\n" \
        "uniform float uPsCharMoved;\n" \
        "uniform vec2 uPsSpecTrans;\n" \
        "uniform float uPsMatSpecPower;\n" \
        "uniform vec3 uPsWaterTint;\n" \
        "uniform float uPsWaterOpacity;\n" \
        "uniform float uPsWavesTimer;\n" \
        "float UnpackDepth(vec4 rgba) {\n" \
        "  return dot(rgba.rgb, vec3(1.0, 1.0 / 255.0, 1.0 / 65025.0));\n" \
        "}\n" \
        "float SampleCrispShadow(sampler2D smap, vec2 uv, float curDepth) {\n" \
        "  if (uv.x < 0.002 || uv.x > 0.998 || uv.y < 0.002 || uv.y > 0.998) return 0.0;\n" \
        "  const float mapSize = 2048.0;\n" \
        "  const float texelSize = 1.0 / 2048.0;\n" \
        "  vec2 texelPos = uv * mapSize - 0.5;\n" \
        "  vec2 f = fract(texelPos);\n" \
        "  vec2 baseUV = (floor(texelPos) + 0.5) * texelSize;\n" \
        "  float s00 = (curDepth > UnpackDepth(textureLod(smap, baseUV, 0.0))) ? 1.0 : 0.0;\n" \
        "  float s10 = (curDepth > UnpackDepth(textureLod(smap, baseUV + vec2(texelSize, 0.0), 0.0))) ? 1.0 : 0.0;\n" \
        "  float s01 = (curDepth > UnpackDepth(textureLod(smap, baseUV + vec2(0.0, texelSize), 0.0))) ? 1.0 : 0.0;\n" \
        "  float s11 = (curDepth > UnpackDepth(textureLod(smap, baseUV + vec2(texelSize, texelSize), 0.0))) ? 1.0 : 0.0;\n" \
        "  return mix(mix(s00, s10, f.x), mix(s01, s11, f.x), f.y);\n" \
        "}\n" \
        "float ComputeCascadeShadow(sampler2D mapBig, int hasBig, sampler2D mapLocal, int hasLocal, float bias1, float bias2, float opNum) {\n" \
        "  vec2 uv2 = vec2(0.5 * vLightPos2.x, -0.5 * vLightPos2.y) + 0.5;\n" \
        "  float borderDist = 0.5 - max(abs(0.5 - uv2.x), abs(0.5 - uv2.y));\n" \
        "  float fade = smoothstep(0.01, 0.12, borderDist);\n" \
        "  float bigShadow = 0.0;\n" \
        "  float localShadow = 0.0;\n" \
        "  if (hasBig != 0 && fade < 0.999) {\n" \
        "    vec2 uv1 = vec2(0.5 * vLightPos1.x, -0.5 * vLightPos1.y) + 0.5;\n" \
        "    float d1 = vLightPos1.z - bias1;\n" \
        "    if (d1 > 0.0 && d1 < 0.999) bigShadow = SampleCrispShadow(mapBig, uv1, d1);\n" \
        "  }\n" \
        "  if (hasLocal != 0 && fade > 0.001) {\n" \
        "    float d2 = vLightPos2.z - bias2;\n" \
        "    if (d2 > 0.0 && d2 < 0.999) localShadow = SampleCrispShadow(mapLocal, uv2, d2);\n" \
        "  }\n" \
        "  float finalShadow = (hasLocal != 0) ? mix(bigShadow, localShadow, fade) : bigShadow;\n" \
        "  float invOpacity = 1.0 - (opNum / 120.0);\n" \
        "  return finalShadow * clamp(invOpacity, 0.25, 0.85);\n" \
        "}\n" \
        "float PuddleRandom(vec2 co) {\n" \
        "  return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);\n" \
        "}\n" \
        "float SmoothNoise(vec2 uvCoord) {\n" \
        "  vec2 i = floor(uvCoord);\n" \
        "  vec2 f = fract(uvCoord);\n" \
        "  f = f * f * (3.0 - 2.0 * f);\n" \
        "  float a = PuddleRandom(i);\n" \
        "  float b = PuddleRandom(i + vec2(1.0, 0.0));\n" \
        "  float c = PuddleRandom(i + vec2(0.0, 1.0));\n" \
        "  float d = PuddleRandom(i + vec2(1.0, 1.0));\n" \
        "  return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);\n" \
        "}\n" \
        "float IsWithinPuddle(vec2 uvCoord, vec3 normal) {\n" \
        "  if (uPsPuddleThreshold >= 1.0) return 0.0;\n" \
        "  float noiseValue = SmoothNoise(uvCoord);\n" \
        "  float spread = mix(0.4, 0.02, clamp(uPsPuddleThreshold, 0.0, 1.0));\n" \
        "  float edgeAA = fwidth(noiseValue);\n" \
        "  float puddlePresence = smoothstep(uPsPuddleThreshold - spread - edgeAA, uPsPuddleThreshold + spread + edgeAA, noiseValue);\n" \
        "  float smoothX = smoothstep(1e-6, 0.15, abs(normal.x));\n" \
        "  float smoothY = smoothstep(1e-6, 0.15, abs(normal.y));\n" \
        "  float isFlatSurface = (smoothX < 1.0 && smoothY < 1.0) ? smoothstep(0.0, 1.0, 1.0 - max(smoothX, smoothY)) : 0.0;\n" \
        "  return puddlePresence * isFlatSurface;\n" \
        "}\n" \
        "vec3 ComputeProceduralNM(sampler2D tex, vec3 centerRgb, vec2 uv, float texRes, float zScale) {\n" \
        "  if (vEyeDist > 3800.0) return vec3(0.5, 0.5, 1.0);\n" \
        "  vec2 ts = vec2(1.0 / texRes);\n" \
        "  vec2 uvFw = fwidth(uv) * texRes;\n" \
        "  float mipAtten = clamp(1.0 / (1.0 + max(uvFw.x, uvFw.y) * 1.2), 0.0, 1.0);\n" \
        "  if (mipAtten < 0.04) return vec3(0.5, 0.5, 1.0);\n" \
        "  vec3 r = texture(tex, uv + vec2(ts.x, 0.0)).rgb;\n" \
        "  vec3 b = texture(tex, uv + vec2(0.0, ts.y)).rgb;\n" \
        "  vec3 dx = (r - centerRgb) * (2.0 * mipAtten);\n" \
        "  vec3 dy = (b - centerRgb) * (2.0 * mipAtten);\n" \
        "  vec3 nm = normalize(vec3(dy.r - dx.r, dy.g - dx.g, zScale));\n" \
        "  return nm * 0.5 + 0.5;\n" \
        "}\n"

    #define UBER_MAIN_BODY(CLIP_DISCARD_STMT, FBO_DISCARD_STMT, TERRAIN_DISCARD_STMT, OBJ_DISCARD_STMT) \
        "void main() {\n" \
        CLIP_DISCARD_STMT \
        "  if (uRenderToFBO != 0 && (uShaderMode == 4 || uShaderMode == 7)) {\n" \
        "    vec4 t0 = (uHasTexture0 != 0) ? texture(uTexture0, vTexCoord0) : vec4(1.0);\n" \
        FBO_DISCARD_STMT \
        "    vec3 L = normalize(-uPsSunDir);\n" \
        "    float nl = clamp(dot(normalize(vWorldNormal), L), 0.0, 1.0);\n" \
        "    vec3 lit = t0.rgb * (uPsAmbient + uPsMatDiffuse * uPsLightDiffuse * nl * 0.65);\n" \
        "    FragColor = vec4(lit, t0.a);\n" \
        "    return;\n" \
        "  }\n" \
        "  if (uShaderMode == 2) {\n" \
        "    float uvDiv = (uPsUVScale > 0.05) ? uPsUVScale : 1.0;\n" \
        "    float calcA = (uHasTexture1 != 0) ? texture(uTexture1, vTexCoord1).a : 1.0;\n" \
        "    float maskA = (uPsAlphaOverride > 0.5) ? 1.0 : ((uvDiv != 1.0) ? clamp(calcA * 1.5, 0.0, 1.0) : calcA);\n" \
        TERRAIN_DISCARD_STMT \
        "    vec2 tiledUV = vTexCoord0 / uvDiv;\n" \
        "    vec4 Color = (uHasTexture0 != 0) ? texture(uTexture0, tiledUV) : vec4(1.0);\n" \
        "    if (uvDiv != 1.0) {\n" \
        "      vec3 NM = ComputeProceduralNM(uTexture0, Color.rgb, tiledUV, 1024.0, 0.15) * 2.0 - 1.0;\n" \
        "      vec3 L = normalize(-uPsSunDir);\n" \
        "      float diffL = clamp(dot(NM, L), 0.0, 1.0);\n" \
        "      Color.rgb *= (0.85 + 0.25 * diffL);\n" \
        "    }\n" \
        "    float isPuddle = (uPsPuddleThreshold < 0.999 && maskA > 0.05) ? IsWithinPuddle(vTexCoord0, vWorldNormal) : 0.0;\n" \
        "    if (isPuddle > 0.001) {\n" \
        "      vec2 ndc = (vScreenPos.xy / max(vScreenPos.w, 0.001));\n" \
        "      vec2 reflection = 0.5 * (1.0 - ndc);\n" \
        "      vec2 foamUV = vec2(0.01) + 0.98 * (vWorldPos.xy * 0.001);\n" \
        "      float tOff = uPsTime * 0.3 * 0.0001975;\n" \
        "      vec3 nm1 = (uHasTexture5 != 0) ? texture(uTexture5, foamUV + vec2(-tOff, 0.0)).rgb : vec3(0.5, 0.5, 1.0);\n" \
        "      vec3 nm2 = (uHasTexture6 != 0) ? texture(uTexture6, foamUV + vec2( tOff, 0.0)).rgb : nm1;\n" \
        "      if (dot(nm1, nm1) < 0.01) nm1 = vec3(0.5, 0.5, 1.0);\n" \
        "      if (dot(nm2, nm2) < 0.01) nm2 = nm1;\n" \
        "      vec3 blendedNormal = normalize((nm1 * 2.0 - 1.0) + (nm2 * 2.0 - 1.0));\n" \
        "      vec2 perturbedUV = reflection + (blendedNormal.xy * 0.5) * 0.0035;\n" \
        "      vec3 seg = uPsCharPos - uPsPrevCharPos;\n" \
        "      float segLen = max(0.001, length(seg));\n" \
        "      float t = clamp(dot(vWorldPos - uPsPrevCharPos, seg) / (segLen * segLen), 0.0, 1.0);\n" \
        "      vec3 interpPos = uPsPrevCharPos + t * seg;\n" \
        "      float distFromPath = distance(vWorldPos, interpPos);\n" \
        "      float inRange1 = clamp(1.0 - (distFromPath / 80.0), 0.0, 1.0);\n" \
        "      float inRange2 = clamp(1.0 - (distFromPath / 300.0), 0.0, 1.0);\n" \
        "      float waveFreq = 0.2 * 3.1416 / 1.7;\n" \
        "      float waveEffect = (uPsCharMoved > 0.1)\n" \
        "        ? (0.006 * inRange2 * sin(distFromPath * (waveFreq / 3.0) - uPsTime * (0.02 / 1.5)))\n" \
        "        : (0.003 * inRange1 * sin((distFromPath / 1.2) * (waveFreq / 2.0) - uPsTime * (0.02 / 3.0)));\n" \
        "      float reflInfluence = clamp(1.0 - (distFromPath / 20000.0), 0.0, 1.0);\n" \
        "      vec2 finalReflUV = clamp(perturbedUV + vec2(waveEffect), 0.002, 0.998);\n" \
        "      vec3 reflCol = (uHasTexture7 != 0) ? texture(uTexture7, finalReflUV).rgb : mix(vec3(0.45, 0.50, 0.58), uFogColor.rgb, 0.5);\n" \
        "      if (dot(reflCol, reflCol) < 0.005) reflCol = mix(vec3(0.45, 0.50, 0.58), uFogColor.rgb, 0.5);\n" \
        "      Color.rgb = mix(Color.rgb, reflCol * 1.15, isPuddle * reflInfluence);\n" \
        "    }\n" \
        "    if (uFogRange.y > uFogRange.x) {\n" \
        "      float fogFactor = clamp((distance(uPsCameraPos, vWorldPos) - uFogRange.x) / (uFogRange.y - uFogRange.x), 0.0, 1.0);\n" \
        "      Color.rgb = mix(Color.rgb, uFogColor.rgb, fogFactor);\n" \
        "    }\n" \
        "    FragColor = vec4(Color.rgb, maskA);\n" \
        "    return;\n" \
        "  }\n" \
        "  if (uShaderMode == 3) {\n" \
        "    vec3 N = normalize(vWorldNormal);\n" \
        "    vec3 L = normalize(-uPsSunDir);\n" \
        "    float origLight = clamp(dot(N, L), 0.0, 1.0);\n" \
        "    float lightIntensity = 0.3 + 0.2 * origLight;\n" \
        "    vec3 lightResult = uPsAmbient + (uPsMatDiffuse * uPsLightDiffuse) * lightIntensity * 0.6;\n" \
        "    float finalShadow = ComputeCascadeShadow(uTexture1, uHasTexture1, uTexture2, uHasTexture2, 0.00035, 0.00005, uPsShadowOpacity);\n" \
        "    lightResult *= (1.0 - finalShadow);\n" \
        "    vec4 outCol = vec4(lightResult, 1.0);\n" \
        "    if (uFogRange.y > uFogRange.x) {\n" \
        "      float fogFactor = clamp((distance(uPsCameraPos, vWorldPos) - uFogRange.x) / (uFogRange.y - uFogRange.x), 0.0, 1.0);\n" \
        "      outCol = mix(outCol, uFogColor, fogFactor);\n" \
        "    }\n" \
        "    FragColor = outCol;\n" \
        "    return;\n" \
        "  }\n" \
        "  if (uShaderMode == 4 || uShaderMode == 7) {\n" \
        "    vec4 Color = (uHasTexture0 != 0) ? texture(uTexture0, vTexCoord0) : vec4(1.0);\n" \
        "    float outA = Color.a * uPsAlphaVal;\n" \
        OBJ_DISCARD_STMT \
        "    vec3 N = normalize(vWorldNormal);\n" \
        "    vec3 L = normalize(-uPsSunDir);\n" \
        "    float lightIntensity = clamp(dot(N, L), 0.0, 1.0);\n" \
        "    float diffMul = (uShaderMode == 7) ? 0.6 : 0.7;\n" \
        "    vec3 lightResult = uPsAmbient + (uPsMatDiffuse * uPsLightDiffuse) * lightIntensity * diffMul;\n" \
        "    float bias1 = (uShaderMode == 7) ? 0.0002 : 0.00004;\n" \
        "    float bias2 = (uShaderMode == 7) ? 0.00004 : 0.00003;\n" \
        "    float finalShadow = ComputeCascadeShadow(uTexture2, uHasTexture2, uTexture3, uHasTexture3, bias1, bias2, 65.0);\n" \
        "    lightResult *= (1.0 - finalShadow);\n" \
        "    if (uShaderMode == 7) {\n" \
        "      vec3 V = normalize(uPsCameraPos - vWorldPos);\n" \
        "      vec3 R = reflect(-L, N);\n" \
        "      float specInt = Color.b * pow(clamp(dot(V, R), 0.0, 1.0), max(uPsShininess * 0.005, 1.0));\n" \
        "      float shFactor = clamp(smoothstep(0.1, 0.5, finalShadow), 0.0, 1.0);\n" \
        "      specInt *= mix(5.5, 0.1, shFactor);\n" \
        "      lightResult += specInt * 0.65 * uPsMatSpecular * uPsSpecularLight;\n" \
        "    }\n" \
        "    if (uShaderMode == 4 && uColorOp1 == 18 && uAlphaTest == 0 && uHasTexture1 != 0 && Color.a < 0.99 && Color.a > 0.01) {\n" \
        "      vec3 dNdx = dFdx(N);\n" \
        "      vec3 dNdy = dFdy(N);\n" \
        "      float specAA = 1.0 / (1.0 + (dot(dNdx, dNdx) + dot(dNdy, dNdy)) * 10.0);\n" \
        "      vec3 V = normalize(uPsCameraPos - vWorldPos);\n" \
        "      vec3 reflVec = reflect(-V, N);\n" \
        "      vec2 specUV = fract(reflVec.xy * 0.5 + 0.5 + uPsSpecTrans.xy * 0.5);\n" \
        "      vec4 specTex = texture(uTexture1, specUV);\n" \
        "      float specInt = clamp(dot(N, V), 0.0, 1.0);\n" \
        "      float spPow = clamp((uPsMatSpecPower > 0.01) ? uPsMatSpecPower : uTexFactor.a, 0.0, 1.0);\n" \
        "      float specFactor = pow(specInt, 0.25) * specTex.a * spPow * specAA;\n" \
        "      Color.rgb = clamp(Color.rgb + specTex.rgb * specFactor * Color.a, 0.0, 1.0);\n" \
        "    }\n" \
        "    Color.rgb *= lightResult;\n" \
        "    if (uFogRange.y > uFogRange.x) {\n" \
        "      float fogFactor = clamp((distance(uPsCameraPos, vWorldPos) - uFogRange.x) / (uFogRange.y - uFogRange.x), 0.0, 1.0);\n" \
        "      Color.rgb = mix(Color.rgb, uFogColor.rgb, fogFactor);\n" \
        "    }\n" \
        "    FragColor = vec4(Color.rgb, outA);\n" \
        "    return;\n" \
        "  }\n" \
        "  vec2 ndc = vScreenPos.xy / max(vScreenPos.w, 0.001);\n" \
        "  vec2 uv = 0.5 * (1.0 - ndc);\n" \
        "  vec2 foamUV = vec2(0.01) + 0.98 * (vWorldPos.xy * 0.001);\n" \
        "  float tOff = uPsWavesTimer * 0.3 * 0.0001975;\n" \
        "  vec2 animUV1 = foamUV + vec2(-tOff, 0.0);\n" \
        "  vec2 animUV2 = foamUV + vec2( tOff, 0.0);\n" \
        "  vec3 nm1 = (uHasTexture4 != 0) ? texture(uTexture4, animUV1).rgb : vec3(0.5, 0.5, 1.0);\n" \
        "  vec3 nm2 = (uHasTexture5 != 0) ? texture(uTexture5, animUV2).rgb : nm1;\n" \
        "  if (dot(nm1, nm1) < 0.01) nm1 = vec3(0.5, 0.5, 1.0);\n" \
        "  if (dot(nm2, nm2) < 0.01) nm2 = nm1;\n" \
        "  vec3 blendedNormal = normalize((nm1 * 2.0 - 1.0) + (nm2 * 2.0 - 1.0));\n" \
        "  float waterDiffPerturb = smoothstep(0.0, 1.0, vColor.a) * 0.8 + 0.2;\n" \
        "  vec2 perturbedUV = clamp(uv + (blendedNormal.xy * 0.5) * (0.004 * waterDiffPerturb), 0.002, 0.998);\n" \
        "  vec2 perturbedUVFoam = animUV1 + (blendedNormal.xy * 0.5) * (0.25 * waterDiffPerturb);\n" \
        "  vec2 waterBaseUV = vTexCoord1 + (blendedNormal.xy * 0.5) * 0.035;\n" \
        "  vec4 waterTexCol = (uHasTexture0 != 0) ? texture(uTexture0, waterBaseUV) : vec4(0.22, 0.42, 0.55, 1.0);\n" \
        "  vec4 reflCol = (uHasTexture3 != 0) ? texture(uTexture3, perturbedUV) : waterTexCol;\n" \
        "  if (uHasTexture3 != 0 && uHasTexture0 != 0) {\n" \
        "    float hasValidRefl = step(0.005, dot(reflCol.rgb, reflCol.rgb));\n" \
        "    reflCol.rgb = mix(waterTexCol.rgb, mix(waterTexCol.rgb, reflCol.rgb, 0.68), hasValidRefl);\n" \
        "  }\n" \
        "  vec4 foamCol = (uHasTexture6 != 0) ? texture(uTexture6, perturbedUVFoam) : waterTexCol;\n" \
        "  foamCol.a = 1.2 * waterDiffPerturb;\n" \
        "  vec3 N = vec3(0.0, 0.0, 1.0);\n" \
        "  vec3 N2 = normalize(vec3(blendedNormal.xy * 0.35, max(abs(blendedNormal.z), 0.85)));\n" \
        "  vec3 L = normalize(-uPsSunDir);\n" \
        "  float lightIntensity = clamp(dot(N, L), 0.0, 1.0);\n" \
        "  vec3 viewDir = normalize(uPsCameraPos - vWorldPos);\n" \
        "  vec3 reflectDir = reflect(-L, N2);\n" \
        "  float spec = pow(clamp(dot(viewDir, reflectDir), 0.0, 1.0), max(uPsShininess * 0.5, 32.0)) / 20.0;\n" \
        "  vec3 specular = spec * uPsSpecularLight * uPsSpecularEnabled * uPsWaterTint;\n" \
        "  vec3 lightResult = uPsAmbient + (uPsMatDiffuse * uPsLightDiffuse) * lightIntensity + specular;\n" \
        "  float finalShadow = ComputeCascadeShadow(uTexture1, uHasTexture1, uTexture2, uHasTexture2, 0.00005, 0.00004, uPsShadowOpacity);\n" \
        "  lightResult *= (1.0 - finalShadow);\n" \
        "  reflCol.rgb *= lightResult;\n" \
        "  float fogFactor = (uFogRange.y > uFogRange.x) ? clamp((distance(uPsCameraPos, vWorldPos) - uFogRange.x) / (uFogRange.y - uFogRange.x), 0.0, 1.0) : 0.0;\n" \
        "  vec4 baseColor = mix(reflCol * 1.15, uFogColor, fogFactor);\n" \
        "  baseColor.rgb *= (1.1 + 1.8 * lightIntensity);\n" \
        "  baseColor.a = 0.78;\n" \
        "  float foamBlendFactor = clamp(1.1 - vColor.a, 0.0, 1.0);\n" \
        "  baseColor.rgb = mix(baseColor.rgb, foamCol.rgb, clamp(foamCol.a * foamBlendFactor * uPsWaterOpacity, 0.0, 0.75));\n" \
        "  vec4 finalColor = mix(baseColor, uFogColor, fogFactor);\n" \
        "  float waterDiffuse = (vColor.a >= 0.999) ? 1.0 : min(vColor.a * 6.0, 1.0);\n" \
        "  FragColor = vec4(finalColor.rgb, clamp(finalColor.a * waterDiffuse, 0.0, 0.92));\n" \
        "}\n"

    // Uber Opaque: ZERO discard statements -> Hardware Early-Z & HSR enabled for Terrain, Buildings, and Water!
    const char* fShaderUberOpaque =
        UBER_FRAGMENT_COMMON
        UBER_MAIN_BODY("", "", "", "");

    // Uber Discard: Used when AlphaTest, ClipPlane, or Terrain Splat Masking is active
    const char* fShaderUberDiscard =
        UBER_FRAGMENT_COMMON
        UBER_MAIN_BODY(
            "  if (uClipPlaneEnable != 0 && dot(vec4(vWorldPos, 1.0), uClipPlane0) < 0.0) discard;\n",
            "    if (uAlphaTest != 0 && t0.a <= uAlphaRef) discard;\n",
            "    if (maskA <= 0.004) discard;\n",
            "    if (uAlphaTest != 0 && outA <= uAlphaRef) discard;\n"
        );

    GLuint CompileShader(GLenum type, const char* source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
        GLint success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            LOGE("Shader Compilation Failed: %s", infoLog);
        }
        return shader;
    }

    enum EProgramVariant {
        PROG_MODE0_OPAQUE = 0,
        PROG_MODE0_DISCARD = 1,
        PROG_MODE1_OPAQUE = 2,
        PROG_MODE1_DISCARD = 3,
        PROG_UBER_OPAQUE = 4,
        PROG_UBER_DISCARD = 5,
        PROG_COUNT = 6
    };

    struct GLProgramInstance {
        GLuint prog = 0;
        GLint uProjectionLoc = -1, uModelViewLoc = -1, uWorldLoc = -1;
        GLint uTexMatrix0Loc = -1, uTexMatrix1Loc = -1;
        GLint uLightViewProj1Loc = -1, uLightViewProj2Loc = -1;
        GLint uTerrainColorMatLoc = -1, uTerrainAlphaMatLoc = -1;
        GLint uUseTexMat0Loc = -1, uUseTexMat1Loc = -1;
        GLint uTexGenMode0Loc = -1, uTexGenMode1Loc = -1;
        GLint uHasNormalLoc = -1, uHasVertexColorLoc = -1, uLightingEnableLoc = -1;
        GLint uShaderModeLoc = -1, uRenderToFBOLoc = -1;
        GLint uClipPlaneEnableLoc = -1, uClipPlane0Loc = -1;
        GLint uLightDirLoc = -1, uLightDiffuseLoc = -1, uLightAmbientLoc = -1;
        GLint uMatDiffuseLoc = -1, uMatAmbientLoc = -1, uMatEmissiveLoc = -1, uGlobalAmbientLoc = -1;
        GLint uHasTexture0Loc = -1, uHasTexture1Loc = -1, uHasTexture2Loc = -1, uHasTexture3Loc = -1;
        GLint uHasTexture4Loc = -1, uHasTexture5Loc = -1, uHasTexture6Loc = -1, uHasTexture7Loc = -1;
        GLint uColorOp0Loc = -1, uColorArg1_0Loc = -1, uColorArg2_0Loc = -1;
        GLint uAlphaOp0Loc = -1, uAlphaArg1_0Loc = -1, uAlphaArg2_0Loc = -1;
        GLint uColorOp1Loc = -1, uColorArg1_1Loc = -1, uColorArg2_1Loc = -1;
        GLint uAlphaOp1Loc = -1, uAlphaArg1_1Loc = -1, uAlphaArg2_1Loc = -1;
        GLint uTexFactorLoc = -1, uAlphaTestLoc = -1, uAlphaRefLoc = -1;
        GLint uFogEnableLoc = -1, uFogColorLoc = -1, uFogRangeLoc = -1;
        GLint uPsMatDiffuseLoc = -1, uPsSunDirLoc = -1, uPsLightDiffuseLoc = -1, uPsAmbientLoc = -1;
        GLint uPsCameraPosLoc = -1, uPsMatSpecularLoc = -1;
        GLint uPsShadowOpacityLoc = -1, uPsSpecularEnabledLoc = -1, uPsShininessLoc = -1, uPsSpecularLightLoc = -1;
        GLint uPsAlphaOverrideLoc = -1, uPsAlphaValLoc = -1;
        GLint uPsTimeLoc = -1, uPsUVScaleLoc = -1, uPsPuddleThresholdLoc = -1;
        GLint uPsCharPosLoc = -1, uPsPrevCharPosLoc = -1, uPsCharMovedLoc = -1;
        GLint uPsSpecTransLoc = -1, uPsMatSpecPowerLoc = -1;
        GLint uPsWaterTintLoc = -1, uPsWaterOpacityLoc = -1, uPsWavesTimerLoc = -1;

        DWORD lastTF = 0xDEADBEEFu;
        DWORD lastAlphaRef = 0xDEADBEEFu;
        DWORD pendingDirtyFlags = 0xFFFFFFFFu;
        int lastState[31];
        int lastClipEn = -1;
        float cModelView[16], cWorld[16], cProj[16], cTexMat0[16], cTexMat1[16];
        float cClipPlane0[4];
        float cLDir[3], cLDiff[3], cLAmb[3], cMDiff[4], cMAmb[3], cMEmi[3], cGAmb[3];
        float cFogCol[4], cFogRng[2];
        float cSpecTrans[2], cMatSpecPower;
        float cLVP1[16], cLVP2[16], cTerColMat[16], cTerAlpMat[16];
        float cPsMatDiff[3], cPsSunDir[3], cPsLightDiff[3], cPsAmb[3], cPsCamPos[3], cPsMatSpec[3], cPsSpecLight[3];
        float cPsShininess, cPsShadowOp, cPsSpecEn, cPsAlphaOvr, cPsTime, cPsUVScale, cPsPuddle;
        float cPsCharPos[3], cPsPrevCharPos[3], cPsCharMoved;
        float cPsAlphaVal, cPsWaterTint[3], cPsWaterOp, cPsWavesTimer;

        void Init(GLuint vShader, const char* fSource) {
            GLuint fShader = CompileShader(GL_FRAGMENT_SHADER, fSource);
            prog = glCreateProgram();
            glAttachShader(prog, vShader);
            glAttachShader(prog, fShader);
            glLinkProgram(prog);
            glDeleteShader(fShader);
			pendingDirtyFlags = 0xFFFFFFFFu;
            for (int i = 0; i < 31; ++i) lastState[i] = -1;

            for (int i = 0; i < 31; ++i) lastState[i] = -1;
            memset(cModelView, 0, sizeof(cModelView));
            memset(cWorld, 0, sizeof(cWorld));
            memset(cProj, 0, sizeof(cProj));
            memset(cTexMat0, 0, sizeof(cTexMat0));
            memset(cTexMat1, 0, sizeof(cTexMat1));
            memset(cClipPlane0, 0, sizeof(cClipPlane0));
            for (int i = 0; i < 3; ++i) {
                cLDir[i] = cLDiff[i] = cLAmb[i] = cMAmb[i] = cMEmi[i] = cGAmb[i] = -999.f;
                cPsMatDiff[i] = cPsSunDir[i] = cPsLightDiff[i] = cPsAmb[i] = cPsCamPos[i] = cPsMatSpec[i] = cPsSpecLight[i] = -999.f;
                cPsCharPos[i] = cPsPrevCharPos[i] = cPsWaterTint[i] = -999.f;
            }
            for (int i = 0; i < 4; ++i) cMDiff[i] = cFogCol[i] = -999.f;
            cFogRng[0] = cFogRng[1] = cSpecTrans[0] = cSpecTrans[1] = -999.f;
            cMatSpecPower = cPsShininess = cPsShadowOp = cPsSpecEn = cPsAlphaOvr = cPsTime = cPsUVScale = cPsPuddle = -999.f;
            cPsCharMoved = cPsAlphaVal = cPsWaterOp = cPsWavesTimer = -999.f;
            memset(cLVP1, 0, sizeof(cLVP1));
            memset(cLVP2, 0, sizeof(cLVP2));
            memset(cTerColMat, 0, sizeof(cTerColMat));
            memset(cTerAlpMat, 0, sizeof(cTerAlpMat));

            uProjectionLoc = glGetUniformLocation(prog, "uProjection");
            uModelViewLoc = glGetUniformLocation(prog, "uModelView");
            uWorldLoc = glGetUniformLocation(prog, "uWorld");
            uTexMatrix0Loc = glGetUniformLocation(prog, "uTexMatrix0");
            uTexMatrix1Loc = glGetUniformLocation(prog, "uTexMatrix1");
            uLightViewProj1Loc = glGetUniformLocation(prog, "uLightViewProj1");
            uLightViewProj2Loc = glGetUniformLocation(prog, "uLightViewProj2");
            uTerrainColorMatLoc = glGetUniformLocation(prog, "uTerrainColorMat");
            uTerrainAlphaMatLoc = glGetUniformLocation(prog, "uTerrainAlphaMat");
            uUseTexMat0Loc = glGetUniformLocation(prog, "uUseTexMat0");
            uUseTexMat1Loc = glGetUniformLocation(prog, "uUseTexMat1");
            uTexGenMode0Loc = glGetUniformLocation(prog, "uTexGenMode0");
            uTexGenMode1Loc = glGetUniformLocation(prog, "uTexGenMode1");
            uHasNormalLoc = glGetUniformLocation(prog, "uHasNormal");
            uHasVertexColorLoc = glGetUniformLocation(prog, "uHasVertexColor");
            uLightingEnableLoc = glGetUniformLocation(prog, "uLightingEnable");
            uShaderModeLoc = glGetUniformLocation(prog, "uShaderMode");
            uRenderToFBOLoc = glGetUniformLocation(prog, "uRenderToFBO");
            uClipPlaneEnableLoc = glGetUniformLocation(prog, "uClipPlaneEnable");
            uClipPlane0Loc = glGetUniformLocation(prog, "uClipPlane0");
            uLightDirLoc = glGetUniformLocation(prog, "uLightDir");
            uLightDiffuseLoc = glGetUniformLocation(prog, "uLightDiffuse");
            uLightAmbientLoc = glGetUniformLocation(prog, "uLightAmbient");
            uMatDiffuseLoc = glGetUniformLocation(prog, "uMatDiffuse");
            uMatAmbientLoc = glGetUniformLocation(prog, "uMatAmbient");
            uMatEmissiveLoc = glGetUniformLocation(prog, "uMatEmissive");
            uGlobalAmbientLoc = glGetUniformLocation(prog, "uGlobalAmbient");
            uHasTexture0Loc = glGetUniformLocation(prog, "uHasTexture0");
            uHasTexture1Loc = glGetUniformLocation(prog, "uHasTexture1");
            uHasTexture2Loc = glGetUniformLocation(prog, "uHasTexture2");
            uHasTexture3Loc = glGetUniformLocation(prog, "uHasTexture3");
            uHasTexture4Loc = glGetUniformLocation(prog, "uHasTexture4");
            uHasTexture5Loc = glGetUniformLocation(prog, "uHasTexture5");
            uHasTexture6Loc = glGetUniformLocation(prog, "uHasTexture6");
            uHasTexture7Loc = glGetUniformLocation(prog, "uHasTexture7");
            uColorOp0Loc = glGetUniformLocation(prog, "uColorOp0");
            uColorArg1_0Loc = glGetUniformLocation(prog, "uColorArg1_0");
            uColorArg2_0Loc = glGetUniformLocation(prog, "uColorArg2_0");
            uAlphaOp0Loc = glGetUniformLocation(prog, "uAlphaOp0");
            uAlphaArg1_0Loc = glGetUniformLocation(prog, "uAlphaArg1_0");
            uAlphaArg2_0Loc = glGetUniformLocation(prog, "uAlphaArg2_0");
            uColorOp1Loc = glGetUniformLocation(prog, "uColorOp1");
            uColorArg1_1Loc = glGetUniformLocation(prog, "uColorArg1_1");
            uColorArg2_1Loc = glGetUniformLocation(prog, "uColorArg2_1");
            uAlphaOp1Loc = glGetUniformLocation(prog, "uAlphaOp1");
            uAlphaArg1_1Loc = glGetUniformLocation(prog, "uAlphaArg1_1");
            uAlphaArg2_1Loc = glGetUniformLocation(prog, "uAlphaArg2_1");
            uTexFactorLoc = glGetUniformLocation(prog, "uTexFactor");
            uAlphaTestLoc = glGetUniformLocation(prog, "uAlphaTest");
            uAlphaRefLoc = glGetUniformLocation(prog, "uAlphaRef");
            uFogEnableLoc = glGetUniformLocation(prog, "uFogEnable");
            uFogColorLoc = glGetUniformLocation(prog, "uFogColor");
            uFogRangeLoc = glGetUniformLocation(prog, "uFogRange");
            uPsMatDiffuseLoc = glGetUniformLocation(prog, "uPsMatDiffuse");
            uPsSunDirLoc = glGetUniformLocation(prog, "uPsSunDir");
            uPsLightDiffuseLoc = glGetUniformLocation(prog, "uPsLightDiffuse");
            uPsAmbientLoc = glGetUniformLocation(prog, "uPsAmbient");
            uPsCameraPosLoc = glGetUniformLocation(prog, "uPsCameraPos");
            uPsMatSpecularLoc = glGetUniformLocation(prog, "uPsMatSpecular");
            uPsShadowOpacityLoc = glGetUniformLocation(prog, "uPsShadowOpacity");
            uPsSpecularEnabledLoc = glGetUniformLocation(prog, "uPsSpecularEnabled");
            uPsShininessLoc = glGetUniformLocation(prog, "uPsShininess");
            uPsSpecularLightLoc = glGetUniformLocation(prog, "uPsSpecularLight");
            uPsAlphaOverrideLoc = glGetUniformLocation(prog, "uPsAlphaOverride");
            uPsAlphaValLoc = glGetUniformLocation(prog, "uPsAlphaVal");
            uPsTimeLoc = glGetUniformLocation(prog, "uPsTime");
            uPsUVScaleLoc = glGetUniformLocation(prog, "uPsUVScale");
            uPsPuddleThresholdLoc = glGetUniformLocation(prog, "uPsPuddleThreshold");
            uPsCharPosLoc = glGetUniformLocation(prog, "uPsCharPos");
            uPsPrevCharPosLoc = glGetUniformLocation(prog, "uPsPrevCharPos");
            uPsCharMovedLoc = glGetUniformLocation(prog, "uPsCharMoved");
            uPsSpecTransLoc = glGetUniformLocation(prog, "uPsSpecTrans");
            uPsMatSpecPowerLoc = glGetUniformLocation(prog, "uPsMatSpecPower");
            uPsWaterTintLoc = glGetUniformLocation(prog, "uPsWaterTint");
            uPsWaterOpacityLoc = glGetUniformLocation(prog, "uPsWaterOpacity");
            uPsWavesTimerLoc = glGetUniformLocation(prog, "uPsWavesTimer");

            glUseProgram(prog);
            const char* samplerNames[8] = {
                "uTexture0", "uTexture1", "uTexture2", "uTexture3",
                "uTexture4", "uTexture5", "uTexture6", "uTexture7"
            };
            for (int i = 0; i < 8; ++i) {
                GLint loc = glGetUniformLocation(prog, samplerNames[i]);
                if (loc >= 0) glUniform1i(loc, i);
            }
        }
    };

    static GLProgramInstance s_progs[PROG_COUNT];
    static GLProgramInstance* s_curProg = NULL;
    static bool s_bProgramsInitialized = false;

    static unsigned int s_enabledAttribMask = 0;
    static GLuint s_curArrayBuffer = 0;
    static GLuint s_curElementArrayBuffer = 0;
    static GLuint s_lastAttrVbo = 0;
    static DWORD s_lastAttrFvf = 0xFFFFFFFFu;
    static UINT s_lastAttrStride = 0xFFFFFFFFu;
    static const BYTE* s_lastAttrBasePtr = (const BYTE*)0xFFFFFFFFu;

    inline void BindArrayBufferCached(GLuint buf) {
        if (s_curArrayBuffer != buf) {
            s_curArrayBuffer = buf;
            glBindBuffer(GL_ARRAY_BUFFER, buf);
        }
    }

    inline void BindElementArrayBufferCached(GLuint buf) {
        if (s_curElementArrayBuffer != buf) {
            s_curElementArrayBuffer = buf;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf);
        }
    }

    inline bool SyncHardwareVertexBuffer(IDirect3DVertexBuffer8* vb) {
        if (!vb || !vb->pData || vb->length == 0 || (vb->usage & D3DUSAGE_DYNAMIC) != 0)
            return false;

        // If a buffer was already uploaded once and is locked again on a subsequent frame,
        // it is a per-frame CPU-deformed mesh (not a static mesh). Promote it to D3DUSAGE_DYNAMIC
        // so we don't stall the mobile GL driver ghosting 4.5 MB/frame via glBufferSubData.
        if (vb->bGpuInitialized && vb->bDirty) {
            vb->usage |= D3DUSAGE_DYNAMIC;
            if (vb->glVbo != 0) {
                NotifyAndroidGLBufferDeleted(vb->glVbo, 0);
                glDeleteBuffers(1, &vb->glVbo);
                vb->glVbo = 0;
            }
            vb->bDirty = false;
            return false;
        }

        if (vb->glVbo == 0)
            glGenBuffers(1, &vb->glVbo);
        if (vb->glVbo == 0)
            return false;

        BindArrayBufferCached(vb->glVbo);

        if (!vb->bGpuInitialized) {
            glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vb->length, vb->pData, GL_STATIC_DRAW);
            vb->bGpuInitialized = true;
            vb->bDirty = false;
            vb->dirtyOffset = vb->length;
            vb->dirtyEnd = 0;
#ifdef ENABLE_MOBILE_PERF_DEBUGGER
            ++g_mobileGLStats.vboUploads;
            g_mobileGLStats.vboUploadBytes += vb->length;
#endif
        }
        return true;
    }

    inline bool SyncHardwareIndexBuffer(IDirect3DIndexBuffer8* ib) {
        if (!ib || !ib->pData || ib->length == 0 || (ib->usage & D3DUSAGE_DYNAMIC) != 0)
            return false;

        if (ib->glIbo == 0)
            glGenBuffers(1, &ib->glIbo);
        if (ib->glIbo == 0)
            return false;

        BindElementArrayBufferCached(ib->glIbo);

        if (ib->bDirty || !ib->bGpuInitialized) {
            UINT off = (ib->dirtyOffset < ib->length) ? ib->dirtyOffset : 0;
            UINT end = (ib->dirtyEnd <= ib->length && ib->dirtyEnd > off) ? ib->dirtyEnd : ib->length;
            UINT subSize = end - off;

            if (!ib->bGpuInitialized) {
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)ib->length, ib->pData, GL_STATIC_DRAW);
                ib->bGpuInitialized = true;
            } else {
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)off, (GLsizeiptr)subSize, (const BYTE*)ib->pData + off);
            }

            ib->bDirty = false;
            ib->dirtyOffset = ib->length;
            ib->dirtyEnd = 0;
        }
        return true;
    }

    static GLuint s_upVbo = 0;
    static size_t s_upVboCapacity = 0;
    static size_t s_upVboOffset = 0;

    static GLuint s_upIbo = 0;
    static size_t s_upIboCapacity = 0;
    static size_t s_upIboOffset = 0;

    inline const BYTE* StreamUPVerticesToHwVbo(const void* pData, size_t byteCount) {
        if (s_upVbo == 0) {
            glGenBuffers(1, &s_upVbo);
            s_upVboCapacity = 1024 * 1024; // 1 MB streaming ring buffer
            BindArrayBufferCached(s_upVbo);
            glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)s_upVboCapacity, NULL, GL_STREAM_DRAW);
            s_upVboOffset = 0;
        } else {
            BindArrayBufferCached(s_upVbo);
        }

        size_t alignedBytes = (byteCount + 15u) & ~15u;
        GLbitfield access = GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;
        if (alignedBytes > s_upVboCapacity) {
            s_upVboCapacity = alignedBytes * 2;
            glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)s_upVboCapacity, NULL, GL_STREAM_DRAW);
            s_upVboOffset = 0;
        } else if (s_upVboOffset + alignedBytes > s_upVboCapacity) {
            s_upVboOffset = 0;
            access |= GL_MAP_INVALIDATE_BUFFER_BIT;
        }

        void* mapped = glMapBufferRange(GL_ARRAY_BUFFER, (GLintptr)s_upVboOffset, (GLsizeiptr)byteCount, access);
        if (mapped) {
            memcpy(mapped, pData, byteCount);
            glUnmapBuffer(GL_ARRAY_BUFFER);
        } else {
            glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)s_upVboOffset, (GLsizeiptr)byteCount, pData);
        }

        const BYTE* gpuOffsetPtr = (const BYTE*)(uintptr_t)s_upVboOffset;
        s_upVboOffset += alignedBytes;
        return gpuOffsetPtr;
    }

    inline const void* StreamUPIndicesToHwIbo(const void* pIdx, size_t byteCount) {
        if (s_upIbo == 0) {
            glGenBuffers(1, &s_upIbo);
            s_upIboCapacity = 256 * 1024; // 256 KB streaming index ring buffer
            BindElementArrayBufferCached(s_upIbo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)s_upIboCapacity, NULL, GL_STREAM_DRAW);
            s_upIboOffset = 0;
        } else {
            BindElementArrayBufferCached(s_upIbo);
        }

        size_t alignedBytes = (byteCount + 15u) & ~15u;
        GLbitfield access = GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;
        if (alignedBytes > s_upIboCapacity) {
            s_upIboCapacity = alignedBytes * 2;
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)s_upIboCapacity, NULL, GL_STREAM_DRAW);
            s_upIboOffset = 0;
        } else if (s_upIboOffset + alignedBytes > s_upIboCapacity) {
            s_upIboOffset = 0;
            access |= GL_MAP_INVALIDATE_BUFFER_BIT;
        }

        void* mapped = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)s_upIboOffset, (GLsizeiptr)byteCount, access);
        if (mapped) {
            memcpy(mapped, pIdx, byteCount);
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
        } else {
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)s_upIboOffset, (GLsizeiptr)byteCount, pIdx);
        }

        const void* gpuOffsetPtr = (const void*)(uintptr_t)s_upIboOffset;
        s_upIboOffset += alignedBytes;
        return gpuOffsetPtr;
    }

    void EnsureProgramInitialized() {
        if (s_bProgramsInitialized) return;
        s_bProgramsInitialized = true;

        GLuint vs0 = CompileShader(GL_VERTEX_SHADER, vShaderMode0);
        GLuint vs1 = CompileShader(GL_VERTEX_SHADER, vShaderMode1);
        GLuint vsUber = CompileShader(GL_VERTEX_SHADER, vShaderUber);

        s_progs[PROG_MODE0_OPAQUE].Init(vs0, fShaderMode0Opaque);
        s_progs[PROG_MODE0_DISCARD].Init(vs0, fShaderMode0Discard);
        s_progs[PROG_MODE1_OPAQUE].Init(vs1, fShaderMode1Opaque);
        s_progs[PROG_MODE1_DISCARD].Init(vs1, fShaderMode1Discard);
        s_progs[PROG_UBER_OPAQUE].Init(vsUber, fShaderUberOpaque);
        s_progs[PROG_UBER_DISCARD].Init(vsUber, fShaderUberDiscard);

        glDeleteShader(vs0);
        glDeleteShader(vs1);
        glDeleteShader(vsUber);

        s_curProg = &s_progs[PROG_MODE0_OPAQUE];
        glUseProgram(s_curProg->prog);

        // Set persistent default attribute values for disabled vertex attribute slots
        glVertexAttrib2f(1, 0.0f, 0.0f);
        glVertexAttrib4f(2, 1.0f, 1.0f, 1.0f, 1.0f);
        glVertexAttrib2f(3, 0.0f, 0.0f);
        glVertexAttrib3f(4, 0.0f, 0.0f, 1.0f);
    }

    inline DWORD ResolveFVF(DWORD dwFVF, UINT stride) {
        DWORD fvf = dwFVF;
        if (D3DXGetFVFVertexSize(fvf) != stride) {
            switch (stride) {
            case 12: fvf = D3DFVF_XYZ; break;
            case 16: fvf = D3DFVF_XYZ | D3DFVF_DIFFUSE; break;
            case 20: fvf = D3DFVF_XYZ | D3DFVF_TEX1; break;
            case 24:
                if (fvf != (D3DFVF_XYZ | D3DFVF_NORMAL))
                    fvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
                break;
            case 32: fvf = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1; break;
            case 36: fvf = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1; break;
            case 40: fvf = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2; break;
            default: break;
            }
        }
        return fvf;
    }

    void BindESVertexAttributes(DWORD dwFVF, const BYTE* basePtr, UINT stride) {
        DWORD fvf = ResolveFVF(dwFVF, stride);

        if (s_curArrayBuffer != 0 &&
            s_lastAttrVbo == s_curArrayBuffer &&
            s_lastAttrFvf == fvf &&
            s_lastAttrStride == stride &&
            s_lastAttrBasePtr == basePtr) {
            return;
        }

        unsigned int wantMask = 1u; // Location 0 (Position) always active

        int offset = 0;
        const BYTE* posPtr = basePtr + offset;
        offset += ((fvf & 0x00E) == D3DFVF_XYZRHW) ? 16 : 12;

        const BYTE* normPtr = NULL;
        if ((fvf & D3DFVF_NORMAL) && (offset + 12 <= (int)stride)) {
            wantMask |= (1u << 4);
            normPtr = basePtr + offset;
            offset += 12;
        }

        if ((fvf & D3DFVF_PSIZE) && (offset + 4 <= (int)stride))
            offset += 4;

        const BYTE* colPtr = NULL;
        if ((fvf & D3DFVF_DIFFUSE) && (offset + 4 <= (int)stride)) {
            wantMask |= (1u << 2);
            colPtr = basePtr + offset;
            offset += 4;
        }

        if ((fvf & D3DFVF_SPECULAR) && (offset + 4 <= (int)stride))
            offset += 4;

        int numTex = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
        const BYTE* uv0Ptr = NULL;
        if ((numTex >= 1 || (fvf & D3DFVF_TEX1)) && (offset + 8 <= (int)stride)) {
            wantMask |= (1u << 1);
            uv0Ptr = basePtr + offset;
            offset += 8;
        }

        const BYTE* uv1Ptr = NULL;
        if ((numTex >= 2 || (fvf & D3DFVF_TEX2)) && (offset + 8 <= (int)stride)) {
            wantMask |= (1u << 3);
            uv1Ptr = basePtr + offset;
        }

        unsigned int diff = wantMask ^ s_enabledAttribMask;
        if (diff) {
            for (GLuint idx = 0; idx < 5; ++idx) {
                unsigned int bit = (1u << idx);
                if (diff & bit) {
                    if (wantMask & bit) glEnableVertexAttribArray(idx);
                    else glDisableVertexAttribArray(idx);
                }
            }
            s_enabledAttribMask = wantMask;
        }

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (GLsizei)stride, posPtr);
        if (uv0Ptr)  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (GLsizei)stride, uv0Ptr);
        if (colPtr)  glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, (GLsizei)stride, colPtr);
        if (uv1Ptr)  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, (GLsizei)stride, uv1Ptr);
        if (normPtr) glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, (GLsizei)stride, normPtr);

        if (s_curArrayBuffer != 0) {
            s_lastAttrVbo = s_curArrayBuffer;
            s_lastAttrFvf = fvf;
            s_lastAttrStride = stride;
            s_lastAttrBasePtr = basePtr;
        } else {
            s_lastAttrVbo = 0;
        }
    }

    inline void UploadVec2IfChanged(GLint loc, float* cache, const float* val) {
        if (loc >= 0 && (cache[0] != val[0] || cache[1] != val[1])) {
            cache[0] = val[0]; cache[1] = val[1];
            glUniform2fv(loc, 1, val);
        }
    }

    inline void UploadVec3IfChanged(GLint loc, float* cache, const float* val) {
        if (loc >= 0 && (cache[0] != val[0] || cache[1] != val[1] || cache[2] != val[2])) {
            cache[0] = val[0]; cache[1] = val[1]; cache[2] = val[2];
            glUniform3fv(loc, 1, val);
        }
    }

    inline void UploadVec4IfChanged(GLint loc, float* cache, const float* val) {
        if (loc >= 0 && (cache[0] != val[0] || cache[1] != val[1] || cache[2] != val[2] || cache[3] != val[3])) {
            cache[0] = val[0]; cache[1] = val[1]; cache[2] = val[2]; cache[3] = val[3];
            glUniform4fv(loc, 1, val);
        }
    }

    inline void UploadFloatIfChanged(GLint loc, float& cache, float val) {
        if (loc >= 0 && cache != val) {
            cache = val;
            glUniform1f(loc, val);
        }
    }

    inline void UploadMat4IfChanged(GLint loc, float* cache, const float* val) {
        if (loc >= 0 && memcmp(cache, val, 16 * sizeof(float)) != 0) {
            memcpy(cache, val, 16 * sizeof(float));
            glUniformMatrix4fv(loc, 1, GL_FALSE, val);
        }
    }

    inline void UploadMat4TransposeIfChanged(GLint loc, float* cache, const float* val) {
        if (loc >= 0 && memcmp(cache, val, 16 * sizeof(float)) != 0) {
            memcpy(cache, val, 16 * sizeof(float));
            glUniformMatrix4fv(loc, 1, GL_TRUE, val);
        }
    }

    inline EProgramVariant SelectProgramVariant(const IDirect3DDevice8* dev, int shaderMode) {
        const bool hasClip = (dev->m_ClipPlaneEnable != 0);
        const bool hasAlphaTest = (dev->m_AlphaTestEnable != 0);

        if (shaderMode == 0) {
            return (hasAlphaTest || hasClip) ? PROG_MODE0_DISCARD : PROG_MODE0_OPAQUE;
        }
        if (shaderMode == 1) {
            return (hasAlphaTest && dev->m_pBoundTextures[0] != NULL) ? PROG_MODE1_DISCARD : PROG_MODE1_OPAQUE;
        }
        // Modes 2..7 (Uber)
        if (hasClip) return PROG_UBER_DISCARD;
        if (shaderMode == 2 && dev->m_psConsts[15][0] <= 0.5f) return PROG_UBER_DISCARD;
        if ((shaderMode == 4 || shaderMode == 7) && hasAlphaTest) return PROG_UBER_DISCARD;
        return PROG_UBER_OPAQUE;
    }

    void SyncESUniforms(IDirect3DDevice8* dev) {
        EnsureProgramInitialized();

        int shaderMode = dev->m_ActivePixelShaderType;
        if (shaderMode == 6) shaderMode = 0;

        EProgramVariant variant = SelectProgramVariant(dev, shaderMode);
        GLProgramInstance* p = &s_progs[variant];

        static UINT s_lastStride = 0xFFFFFFFFu;
        if (dev->m_StreamStride != s_lastStride) {
            s_lastStride = dev->m_StreamStride;
            dev->m_DirtyFlags |= 4;
        }

        // Distribute newly dirtied device flags to ALL 6 program variants before clearing dev->m_DirtyFlags
        if (dev->m_DirtyFlags != 0) {
            for (int i = 0; i < PROG_COUNT; ++i) {
                s_progs[i].pendingDirtyFlags |= dev->m_DirtyFlags;
            }
            dev->m_DirtyFlags = 0;
        }

        if (s_curProg != p) {
            s_curProg = p;
            glUseProgram(p->prog);
#ifdef ENABLE_MOBILE_PERF_DEBUGGER
            ++g_mobileGLStats.progSwitches;
#endif
        }

        // Fast-check TextureFactor and AlphaRef first (frequently updated between particles/foliage)
        if (p->uTexFactorLoc >= 0 && dev->m_TextureFactor != p->lastTF) {
            p->lastTF = dev->m_TextureFactor;
            GLCOLOR tf(dev->m_TextureFactor);
            glUniform4f(p->uTexFactorLoc, tf.r, tf.g, tf.b, tf.a);
        }

        if (p->uAlphaRefLoc >= 0 && dev->m_AlphaRef != p->lastAlphaRef) {
            p->lastAlphaRef = dev->m_AlphaRef;
            glUniform1f(p->uAlphaRefLoc, (float)dev->m_AlphaRef / 255.0f);
        }

        DWORD dirty = p->pendingDirtyFlags;
        if (dirty == 0)
            return;
        p->pendingDirtyFlags = 0;

        if (dirty & 1) {
            GLMATRIX mv;
            D3DXMatrixMultiply(&mv, &dev->m_matWorld, &dev->m_matView);
            UploadMat4IfChanged(p->uModelViewLoc, p->cModelView, &mv.m[0][0]);
            UploadMat4IfChanged(p->uWorldLoc, p->cWorld, &dev->m_matWorld.m[0][0]);
        }

        if (dirty & 2) {
            UploadMat4IfChanged(p->uProjectionLoc, p->cProj, &dev->m_matProj.m[0][0]);
        }

        int renderToFBO = (dev->m_pCurrentRenderTargetTex != NULL) ? 1 : 0;

        if (dirty & 4) {
            if (shaderMode == 1) {
                if (p->uRenderToFBOLoc >= 0 && p->lastState[24] != renderToFBO) {
                    p->lastState[24] = renderToFBO;
                    glUniform1i(p->uRenderToFBOLoc, renderToFBO);
                }
                return;
            }

            DWORD fvf = ResolveFVF(dev->m_dwFVF, dev->m_StreamStride);
            int hasNormal = (fvf & D3DFVF_NORMAL) ? 1 : 0;
            int hasVertexColor = ((fvf & D3DFVF_DIFFUSE) && dev->m_ColorVertex) ? 1 : 0;

            int useTexMat0 = (dev->m_TexTransformFlags[0] != D3DTTFF_DISABLE) ? 1 : 0;
            int useTexMat1 = (dev->m_TexTransformFlags[1] != D3DTTFF_DISABLE) ? 1 : 0;
            if (useTexMat0) UploadMat4IfChanged(p->uTexMatrix0Loc, p->cTexMat0, &dev->m_matTex[0].m[0][0]);
            if (useTexMat1) UploadMat4IfChanged(p->uTexMatrix1Loc, p->cTexMat1, &dev->m_matTex[1].m[0][0]);

            int tciMode0 = (int)((dev->m_TexCoordIndex[0] >> 16) & 0xF);
            int tciMode1 = (int)((dev->m_TexCoordIndex[1] >> 16) & 0xF);

            int state[31] = {
                useTexMat0, useTexMat1,
                tciMode0, tciMode1,
                (dev->m_pBoundTextures[0] != NULL) ? 1 : 0,
                (dev->m_pBoundTextures[1] != NULL) ? 1 : 0,
                (int)dev->m_ColorOp[0], (int)dev->m_ColorArg1[0], (int)dev->m_ColorArg2[0],
                (int)dev->m_AlphaOp[0], (int)dev->m_AlphaArg1[0], (int)dev->m_AlphaArg2[0],
                (int)dev->m_ColorOp[1], (int)dev->m_ColorArg1[1], (int)dev->m_ColorArg2[1],
                (int)dev->m_AlphaOp[1], (int)dev->m_AlphaArg1[1], (int)dev->m_AlphaArg2[1],
                dev->m_AlphaTestEnable ? 1 : 0,
                hasNormal, hasVertexColor,
                (dev->m_LightingEnable && hasNormal) ? 1 : 0,
                (dev->m_FogEnable && ((fvf & 0x00E) != D3DFVF_XYZRHW)) ? 1 : 0,
                shaderMode, renderToFBO,
                (dev->m_pBoundTextures[2] != NULL) ? 1 : 0,
                (dev->m_pBoundTextures[3] != NULL) ? 1 : 0,
                (dev->m_pBoundTextures[4] != NULL) ? 1 : 0,
                (dev->m_pBoundTextures[5] != NULL) ? 1 : 0,
                (dev->m_pBoundTextures[6] != NULL) ? 1 : 0,
                (dev->m_pBoundTextures[7] != NULL) ? 1 : 0
            };
            const GLint locs[31] = {
                p->uUseTexMat0Loc, p->uUseTexMat1Loc, p->uTexGenMode0Loc, p->uTexGenMode1Loc,
                p->uHasTexture0Loc, p->uHasTexture1Loc,
                p->uColorOp0Loc, p->uColorArg1_0Loc, p->uColorArg2_0Loc,
                p->uAlphaOp0Loc, p->uAlphaArg1_0Loc, p->uAlphaArg2_0Loc,
                p->uColorOp1Loc, p->uColorArg1_1Loc, p->uColorArg2_1Loc,
                p->uAlphaOp1Loc, p->uAlphaArg1_1Loc, p->uAlphaArg2_1Loc,
                p->uAlphaTestLoc,
                p->uHasNormalLoc, p->uHasVertexColorLoc, p->uLightingEnableLoc, p->uFogEnableLoc,
                p->uShaderModeLoc, p->uRenderToFBOLoc,
                p->uHasTexture2Loc, p->uHasTexture3Loc,
                p->uHasTexture4Loc, p->uHasTexture5Loc,
                p->uHasTexture6Loc, p->uHasTexture7Loc
            };
            for (int i = 0; i < 31; ++i) {
                if (locs[i] >= 0 && state[i] != p->lastState[i]) {
                    p->lastState[i] = state[i];
                    glUniform1i(locs[i], state[i]);
                }
            }

            int clipEn = (dev->m_ClipPlaneEnable != 0) ? 1 : 0;
            if (p->uClipPlaneEnableLoc >= 0 && clipEn != p->lastClipEn) {
                p->lastClipEn = clipEn;
                glUniform1i(p->uClipPlaneEnableLoc, clipEn);
            }
            if (clipEn) {
                UploadVec4IfChanged(p->uClipPlane0Loc, p->cClipPlane0, dev->m_ClipPlane0);
            }

            if (state[21] || shaderMode == 4 || shaderMode == 7) {
                UploadVec4IfChanged(p->uMatDiffuseLoc, p->cMDiff, &dev->m_Material.Diffuse.r);
            }

            if (state[21] && shaderMode == 0) {
                GLVECTOR3 ldir = dev->m_Light0.Direction;
                float lLen = D3DXVec3Length(&ldir);
                if (lLen > 1e-4f) { ldir.x /= lLen; ldir.y /= lLen; ldir.z /= lLen; }
                else { ldir = GLVECTOR3(0.0f, 0.0f, -1.0f); }
                UploadVec3IfChanged(p->uLightDirLoc, p->cLDir, &ldir.x);
                UploadVec3IfChanged(p->uLightDiffuseLoc, p->cLDiff, &dev->m_Light0.Diffuse.r);
                UploadVec3IfChanged(p->uLightAmbientLoc, p->cLAmb, &dev->m_Light0.Ambient.r);
                UploadVec3IfChanged(p->uMatAmbientLoc, p->cMAmb, &dev->m_Material.Ambient.r);
                UploadVec3IfChanged(p->uMatEmissiveLoc, p->cMEmi, &dev->m_Material.Emissive.r);
                GLCOLOR gAmb(dev->m_AmbientColor);
                UploadVec3IfChanged(p->uGlobalAmbientLoc, p->cGAmb, &gAmb.r);
            }

            if (state[22] && shaderMode == 0) {
                GLCOLOR fc(dev->m_FogColor);
                float fr[2] = { dev->m_FogStart, dev->m_FogEnd };
                UploadVec4IfChanged(p->uFogColorLoc, p->cFogCol, &fc.r);
                UploadVec2IfChanged(p->uFogRangeLoc, p->cFogRng, fr);
            }
        }

        if (dirty & 12) {
            if (dev->m_ColorOp[1] == 18 || shaderMode == 4) {
                UploadVec2IfChanged(p->uPsSpecTransLoc, p->cSpecTrans, &dev->m_psConsts[91][0]);
                UploadFloatIfChanged(p->uPsMatSpecPowerLoc, p->cMatSpecPower, dev->m_psConsts[92][0]);
            }

            if (shaderMode >= 2) {
                UploadMat4TransposeIfChanged(p->uLightViewProj1Loc, p->cLVP1, &dev->m_vsConsts[8][0]);
                UploadMat4TransposeIfChanged(p->uLightViewProj2Loc, p->cLVP2, &dev->m_vsConsts[16][0]);
                UploadVec3IfChanged(p->uPsMatDiffuseLoc, p->cPsMatDiff, &dev->m_psConsts[0][0]);
                UploadVec3IfChanged(p->uPsSunDirLoc, p->cPsSunDir, &dev->m_psConsts[1][0]);
                UploadVec3IfChanged(p->uPsLightDiffuseLoc, p->cPsLightDiff, &dev->m_psConsts[2][0]);
                UploadVec3IfChanged(p->uPsAmbientLoc, p->cPsAmb, &dev->m_psConsts[3][0]);
                UploadVec3IfChanged(p->uPsCameraPosLoc, p->cPsCamPos, &dev->m_psConsts[4][0]);
                UploadVec4IfChanged(p->uFogColorLoc, p->cFogCol, &dev->m_psConsts[5][0]);
                UploadVec2IfChanged(p->uFogRangeLoc, p->cFogRng, &dev->m_psConsts[6][0]);
                UploadVec3IfChanged(p->uPsMatSpecularLoc, p->cPsMatSpec, &dev->m_psConsts[7][0]);
                UploadFloatIfChanged(p->uPsShininessLoc, p->cPsShininess, dev->m_psConsts[8][0]);
                UploadVec3IfChanged(p->uPsSpecularLightLoc, p->cPsSpecLight, &dev->m_psConsts[9][0]);
                float rawOp = dev->m_psConsts[10][0];
                if (rawOp < 1.0f || rawOp > 120.0f) rawOp = (float)(*(const int*)&dev->m_psConsts[10][0]);
                if (rawOp < 10.0f || rawOp > 110.0f) rawOp = 55.0f;
                UploadFloatIfChanged(p->uPsShadowOpacityLoc, p->cPsShadowOp, rawOp);
                UploadFloatIfChanged(p->uPsSpecularEnabledLoc, p->cPsSpecEn, dev->m_psConsts[11][0]);
                if (shaderMode == 2) {
                    UploadMat4TransposeIfChanged(p->uTerrainColorMatLoc, p->cTerColMat, &dev->m_vsConsts[24][0]);
                    UploadMat4TransposeIfChanged(p->uTerrainAlphaMatLoc, p->cTerAlpMat, &dev->m_vsConsts[28][0]);
                    UploadFloatIfChanged(p->uPsAlphaOverrideLoc, p->cPsAlphaOvr, dev->m_psConsts[15][0]);
                    UploadFloatIfChanged(p->uPsTimeLoc, p->cPsTime, dev->m_psConsts[12][0]);
                    UploadFloatIfChanged(p->uPsUVScaleLoc, p->cPsUVScale, dev->m_psConsts[17][0]);
                    UploadVec3IfChanged(p->uPsCharPosLoc, p->cPsCharPos, &dev->m_psConsts[86][0]);
                    UploadFloatIfChanged(p->uPsPuddleThresholdLoc, p->cPsPuddle, dev->m_psConsts[87][0]);
                    UploadVec3IfChanged(p->uPsPrevCharPosLoc, p->cPsPrevCharPos, &dev->m_psConsts[93][0]);
                    UploadFloatIfChanged(p->uPsCharMovedLoc, p->cPsCharMoved, dev->m_psConsts[94][0]);
                }
                if (shaderMode == 4 || shaderMode == 7) {
                    float aVal = dev->m_psConsts[83][0];
                    if (aVal <= 0.001f || aVal > 1.0f) aVal = 1.0f;
                    UploadFloatIfChanged(p->uPsAlphaValLoc, p->cPsAlphaVal, aVal);
                }
                if (shaderMode == 5) {
                    UploadVec3IfChanged(p->uPsWaterTintLoc, p->cPsWaterTint, &dev->m_psConsts[12][0]);
                    UploadFloatIfChanged(p->uPsWaterOpacityLoc, p->cPsWaterOp, dev->m_psConsts[72][0]);
                    UploadFloatIfChanged(p->uPsWavesTimerLoc, p->cPsWavesTimer, dev->m_psConsts[73][0]);
                }
            }
        }
    }
} // anonymous namespace

void NotifyAndroidGLBufferDeleted(GLuint vbo, GLuint ibo) {
    if (vbo != 0) {
        if (s_curArrayBuffer == vbo) s_curArrayBuffer = 0;
        if (s_lastAttrVbo == vbo) {
            s_lastAttrVbo = 0;
            s_lastAttrFvf = 0xFFFFFFFFu;
        }
    }
    if (ibo != 0) {
        if (s_curElementArrayBuffer == ibo) s_curElementArrayBuffer = 0;
    }
}

IDirect3D8* Direct3DCreate8(UINT SDKVersion) {
    return new IDirect3D8();
}

HRESULT IDirect3D8::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface) {
    IDirect3DDevice8* dev = new IDirect3DDevice8();
    if (pPresentationParameters && pPresentationParameters->BackBufferWidth > 0 && pPresentationParameters->BackBufferHeight > 0) {
        dev->m_Viewport.X = 0;
        dev->m_Viewport.Y = 0;
        dev->m_Viewport.Width = pPresentationParameters->BackBufferWidth;
        dev->m_Viewport.Height = pPresentationParameters->BackBufferHeight;
        dev->m_ScreenViewport = dev->m_Viewport;
        glViewport(0, 0,
            (g_AndroidPhysWidth > 0) ? g_AndroidPhysWidth : dev->m_Viewport.Width,
            (g_AndroidPhysHeight > 0) ? g_AndroidPhysHeight : dev->m_Viewport.Height);
    }
    *ppReturnedDeviceInterface = dev;
    EnsureProgramInitialized();
    return S_OK;
}

HRESULT IDirect3DDevice8::SetRenderTarget(LPDIRECT3DSURFACE8 pRenderTarget, LPDIRECT3DSURFACE8) {
    static GLint s_defaultScreenFbo = 0;
    if (pRenderTarget && pRenderTarget->pParentTex) {
        if (!m_pCurrentRenderTargetTex) {
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &s_defaultScreenFbo);
        }
        IDirect3DTexture8* tex = pRenderTarget->pParentTex;
        if (!tex->glFbo) {
            glGenFramebuffers(1, &tex->glFbo);
            glBindFramebuffer(GL_FRAMEBUFFER, tex->glFbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->glId, 0);
            glGenRenderbuffers(1, &tex->glDepthRbo);
            glBindRenderbuffer(GL_RENDERBUFFER, tex->glDepthRbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, tex->width, tex->height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, tex->glDepthRbo);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, tex->glFbo);
        }
        m_pCurrentRenderTargetTex = tex;
        glViewport(0, 0, tex->width, tex->height);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)s_defaultScreenFbo);
        m_pCurrentRenderTargetTex = NULL;
        m_Viewport = m_ScreenViewport;
        glViewport(0, 0,
            (g_AndroidPhysWidth > 0) ? g_AndroidPhysWidth : m_ScreenViewport.Width,
            (g_AndroidPhysHeight > 0) ? g_AndroidPhysHeight : m_ScreenViewport.Height);
    }
#ifdef ENABLE_MOBILE_PERF_DEBUGGER
    ++g_mobileGLStats.fboSwitches;
#endif
    m_DirtyFlags |= 4;
    return S_OK;
}

HRESULT IDirect3DDevice8::Clear(DWORD Count, const D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
    GLbitfield mask = 0;
    if (Flags & D3DCLEAR_TARGET) {
        GLCOLOR col(Color);
        glClearColor(col.r, col.g, col.b, col.a);
        mask |= GL_COLOR_BUFFER_BIT;
    }
    if (Flags & D3DCLEAR_ZBUFFER) {
        glClearDepthf(Z);
        mask |= GL_DEPTH_BUFFER_BIT;
    }
    if (mask) glClear(mask);
    return S_OK;
}

HRESULT IDirect3DDevice8::DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount) {
    if (!m_pStreamSource || !m_pIndexBuffer || !m_pStreamSource->pData || !m_pIndexBuffer->pData || m_StreamStride == 0 || PrimitiveCount == 0)
        return S_OK;

    GLenum mode = GL_TRIANGLES;
    GLsizei count = 0;
    switch (Type) {
    case D3DPT_POINTLIST: mode = GL_POINTS; count = (GLsizei)PrimitiveCount; break;
    case D3DPT_LINELIST: mode = GL_LINES; count = (GLsizei)PrimitiveCount * 2; break;
    case D3DPT_LINESTRIP: mode = GL_LINE_STRIP; count = (GLsizei)PrimitiveCount + 1; break;
    case D3DPT_TRIANGLELIST: mode = GL_TRIANGLES; count = (GLsizei)PrimitiveCount * 3; break;
    case D3DPT_TRIANGLESTRIP: mode = GL_TRIANGLE_STRIP; count = (GLsizei)PrimitiveCount + 2; break;
    case D3DPT_TRIANGLEFAN: mode = GL_TRIANGLE_FAN; count = (GLsizei)PrimitiveCount + 2; break;
    default: return S_OK;
    }

    if (m_BaseVertexIndex > 0x00FFFFFFu)
        return S_OK;
    size_t vOffsetBytes = (size_t)m_BaseVertexIndex * (size_t)m_StreamStride;
    if (vOffsetBytes >= (size_t)m_pStreamSource->length)
        return S_OK;
    if (((size_t)StartIndex + (size_t)count) * sizeof(WORD) > (size_t)m_pIndexBuffer->length)
        return S_OK;

#ifdef ENABLE_MOBILE_PERF_DEBUGGER
    const uint64_t t0 = GetPerfTimeNs();
    const int bucket = ClassifyDrawBucket(this, false);
#endif

    SyncESUniforms(this);

    const bool useHwVbo = SyncHardwareVertexBuffer(m_pStreamSource);
    const BYTE* basePtr = (useHwVbo ? (const BYTE*)(uintptr_t)0 : (const BYTE*)m_pStreamSource->pData) + vOffsetBytes;
    if (!useHwVbo) {
        BindArrayBufferCached(0);
    }
    BindESVertexAttributes(m_dwFVF, basePtr, m_StreamStride);

    ++GetAndroidQueryDrawCounter();

    const bool useHwIbo = SyncHardwareIndexBuffer(m_pIndexBuffer);
    if (useHwIbo) {
        glDrawElements(mode, count, GL_UNSIGNED_SHORT, (const void*)((size_t)StartIndex * sizeof(WORD)));
    } else {
        BindElementArrayBufferCached(0);
        const WORD* idxPtr = (const WORD*)m_pIndexBuffer->pData + StartIndex;
        glDrawElements(mode, count, GL_UNSIGNED_SHORT, idxPtr);
    }

#ifdef ENABLE_MOBILE_PERF_DEBUGGER
    g_mobileGLStats.drawCalls[bucket]++;
    g_mobileGLStats.tris[bucket] += PrimitiveCount;
    g_mobileGLStats.cpuTimeNs[bucket] += (GetPerfTimeNs() - t0);
#endif
    return S_OK;
}

HRESULT IDirect3DDevice8::DrawPrimitive(D3DPRIMITIVETYPE Type, UINT StartVertex, UINT PrimitiveCount) {
    if (!m_pStreamSource || !m_pStreamSource->pData || m_StreamStride == 0 || PrimitiveCount == 0)
        return S_OK;

    GLenum mode = GL_TRIANGLES;
    GLsizei count = 0;
    switch (Type) {
    case D3DPT_POINTLIST: mode = GL_POINTS; count = (GLsizei)PrimitiveCount; break;
    case D3DPT_LINELIST: mode = GL_LINES; count = (GLsizei)PrimitiveCount * 2; break;
    case D3DPT_LINESTRIP: mode = GL_LINE_STRIP; count = (GLsizei)PrimitiveCount + 1; break;
    case D3DPT_TRIANGLELIST: mode = GL_TRIANGLES; count = (GLsizei)PrimitiveCount * 3; break;
    case D3DPT_TRIANGLESTRIP: mode = GL_TRIANGLE_STRIP; count = (GLsizei)PrimitiveCount + 2; break;
    case D3DPT_TRIANGLEFAN: mode = GL_TRIANGLE_FAN; count = (GLsizei)PrimitiveCount + 2; break;
    default: return S_OK;
    }

    if ((size_t)(StartVertex + count) * (size_t)m_StreamStride > (size_t)m_pStreamSource->length)
        return S_OK;

#ifdef ENABLE_MOBILE_PERF_DEBUGGER
    const uint64_t t0 = GetPerfTimeNs();
    const int bucket = ClassifyDrawBucket(this, false);
#endif

    SyncESUniforms(this);

    const bool useHwVbo = SyncHardwareVertexBuffer(m_pStreamSource);
    const BYTE* basePtr = useHwVbo ? (const BYTE*)(uintptr_t)0 : (const BYTE*)m_pStreamSource->pData;
    if (!useHwVbo) {
        BindArrayBufferCached(0);
    }
    BindESVertexAttributes(m_dwFVF, basePtr, m_StreamStride);

    ++GetAndroidQueryDrawCounter();
    glDrawArrays(mode, (GLint)StartVertex, count);

#ifdef ENABLE_MOBILE_PERF_DEBUGGER
    g_mobileGLStats.drawCalls[bucket]++;
    g_mobileGLStats.tris[bucket] += PrimitiveCount;
    g_mobileGLStats.cpuTimeNs[bucket] += (GetPerfTimeNs() - t0);
#endif
    return S_OK;
}

HRESULT IDirect3DDevice8::DrawPrimitiveUP(D3DPRIMITIVETYPE Type, UINT PrimitiveCount, const void* pData, UINT Stride) {
    if (!pData || Stride == 0 || PrimitiveCount == 0)
        return S_OK;

    GLenum mode = GL_TRIANGLES;
    GLsizei count = 0;
    switch (Type) {
    case D3DPT_POINTLIST: mode = GL_POINTS; count = (GLsizei)PrimitiveCount; break;
    case D3DPT_LINELIST: mode = GL_LINES; count = (GLsizei)PrimitiveCount * 2; break;
    case D3DPT_LINESTRIP: mode = GL_LINE_STRIP; count = (GLsizei)PrimitiveCount + 1; break;
    case D3DPT_TRIANGLELIST: mode = GL_TRIANGLES; count = (GLsizei)PrimitiveCount * 3; break;
    case D3DPT_TRIANGLESTRIP: mode = GL_TRIANGLE_STRIP; count = (GLsizei)PrimitiveCount + 2; break;
    case D3DPT_TRIANGLEFAN: mode = GL_TRIANGLE_FAN; count = (GLsizei)PrimitiveCount + 2; break;
    default: return S_OK;
    }

#ifdef ENABLE_MOBILE_PERF_DEBUGGER
    const uint64_t t0 = GetPerfTimeNs();
    const int bucket = ClassifyDrawBucket(this, true);
#endif

    m_StreamStride = Stride;
    SyncESUniforms(this);
    BindArrayBufferCached(0);
    BindESVertexAttributes(m_dwFVF, (const BYTE*)pData, Stride);
    glDrawArrays(mode, 0, count);

#ifdef ENABLE_MOBILE_PERF_DEBUGGER
    g_mobileGLStats.drawCalls[bucket]++;
    g_mobileGLStats.tris[bucket] += PrimitiveCount;
    g_mobileGLStats.cpuTimeNs[bucket] += (GetPerfTimeNs() - t0);
#endif
    return S_OK;
}

HRESULT IDirect3DDevice8::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE Type, UINT, UINT, UINT PrimitiveCount, const void* pIdx, D3DFORMAT IdxFmt, const void* pData, UINT Stride) {
    if (!pData || !pIdx || Stride == 0 || PrimitiveCount == 0)
        return S_OK;

    GLenum mode = GL_TRIANGLES;
    GLsizei count = 0;
    switch (Type) {
    case D3DPT_POINTLIST: mode = GL_POINTS; count = (GLsizei)PrimitiveCount; break;
    case D3DPT_LINELIST: mode = GL_LINES; count = (GLsizei)PrimitiveCount * 2; break;
    case D3DPT_LINESTRIP: mode = GL_LINE_STRIP; count = (GLsizei)PrimitiveCount + 1; break;
    case D3DPT_TRIANGLELIST: mode = GL_TRIANGLES; count = (GLsizei)PrimitiveCount * 3; break;
    case D3DPT_TRIANGLESTRIP: mode = GL_TRIANGLE_STRIP; count = (GLsizei)PrimitiveCount + 2; break;
    case D3DPT_TRIANGLEFAN: mode = GL_TRIANGLE_FAN; count = (GLsizei)PrimitiveCount + 2; break;
    default: return S_OK;
    }

#ifdef ENABLE_MOBILE_PERF_DEBUGGER
    const uint64_t t0 = GetPerfTimeNs();
    const int bucket = ClassifyDrawBucket(this, true);
#endif

    m_StreamStride = Stride;
    SyncESUniforms(this);
    BindArrayBufferCached(0);
    BindElementArrayBufferCached(0);
    BindESVertexAttributes(m_dwFVF, (const BYTE*)pData, Stride);
    glDrawElements(mode, count, (IdxFmt == D3DFMT_INDEX16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT), pIdx);

#ifdef ENABLE_MOBILE_PERF_DEBUGGER
    g_mobileGLStats.drawCalls[bucket]++;
    g_mobileGLStats.tris[bucket] += PrimitiveCount;
    g_mobileGLStats.cpuTimeNs[bucket] += (GetPerfTimeNs() - t0);
#endif
    return S_OK;
}

HRESULT IDirect3DDevice8::Reset(D3DPRESENT_PARAMETERS* pPP) {
    if (pPP && pPP->BackBufferWidth > 0 && pPP->BackBufferHeight > 0) {
        m_Viewport.X = 0;
        m_Viewport.Y = 0;
        m_Viewport.Width = pPP->BackBufferWidth;
        m_Viewport.Height = pPP->BackBufferHeight;
        m_ScreenViewport = m_Viewport;
        glViewport(0, 0,
            (g_AndroidPhysWidth > 0) ? g_AndroidPhysWidth : m_Viewport.Width,
            (g_AndroidPhysHeight > 0) ? g_AndroidPhysHeight : m_Viewport.Height);
    }
    m_DirtyFlags = 0xFFFFFFFFu;
    return S_OK;
}

HRESULT IDirect3DDevice8::BeginScene() { return S_OK; }
HRESULT IDirect3DDevice8::EndScene() { return S_OK; }
HRESULT IDirect3DDevice8::Present(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const void* pDirtyRegion) { return S_OK; }

HRESULT IDirect3DDevice8::SetTexture(DWORD Stage, LPDIRECT3DBaseTexture8 pTexture) {
    if (Stage >= 16) return S_OK;
    if (pTexture && (IDirect3DTexture8*)pTexture == m_pCurrentRenderTargetTex) pTexture = NULL;
    LPDIRECT3DBaseTexture8 prevTex = m_pBoundTextures[Stage];
    if (prevTex != pTexture) {
        m_pBoundTextures[Stage] = pTexture;
        if (Stage < 8 && ((prevTex != NULL) != (pTexture != NULL))) {
            m_DirtyFlags |= 4;
        }
    }
    if (Stage >= 8) return S_OK;

    IDirect3DTexture8* tex = (IDirect3DTexture8*)pTexture;
    GLuint newGlId = tex ? tex->glId : 0;
    DWORD addrU = m_AddressU[Stage];
    DWORD addrV = m_AddressV[Stage];
    GLint wantS = (addrU == D3DTADDRESS_CLAMP || addrU == D3DTADDRESS_BORDER) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
    GLint wantT = (addrV == D3DTADDRESS_CLAMP || addrV == D3DTADDRESS_BORDER) ? GL_CLAMP_TO_EDGE : GL_REPEAT;

    if (m_BoundGLTexId[Stage] == newGlId && (!tex || (tex->lastWrapS == wantS && tex->lastWrapT == wantT)))
        return S_OK;

    m_BoundGLTexId[Stage] = newGlId;
#ifdef ENABLE_MOBILE_PERF_DEBUGGER
    ++g_mobileGLStats.texBinds;
#endif
    glActiveTexture(GL_TEXTURE0 + Stage);
    glBindTexture(GL_TEXTURE_2D, newGlId);
    if (tex) {
        if (tex->lastWrapS != wantS) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wantS);
            tex->lastWrapS = wantS;
        }
        if (tex->lastWrapT != wantT) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wantT);
            tex->lastWrapT = wantT;
        }
    }
    glActiveTexture(GL_TEXTURE0);
    return S_OK;
}

HRESULT IDirect3DDevice8::SetStreamSource(UINT StreamNumber, LPDIRECT3DVERTEXBUFFER8 pStreamData, UINT Stride) {
    m_pStreamSource = pStreamData;
    if (m_StreamStride != Stride) {
        m_StreamStride = Stride;
        m_DirtyFlags |= 4;
    }
    return S_OK;
}

HRESULT IDirect3DDevice8::SetIndices(LPDIRECT3DINDEXBUFFER8 pIndexBuffer, UINT BaseVertexIndex) {
    m_pIndexBuffer = pIndexBuffer;
    m_BaseVertexIndex = BaseVertexIndex;
    return S_OK;
}

#endif