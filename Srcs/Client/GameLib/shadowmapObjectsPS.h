// ============================================================================
// MODE 3: Original 30-Sample PCF & 9-Tap Sobel Object Shader (from BEFORE)
// ============================================================================
const char* shadowmapObjectsPS_Mode3 = R"(
struct PS_INPUT
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float4 LightViewPosition1 : TEXCOORD2;
    float4 LightViewPosition2 : TEXCOORD3;
    float4 WorldPos : TEXCOORD4;
    float3 Tangent : TEXCOORD5;
};

#define USE_SHADOWS
#define SHADOW_PCF_RADIUS 2
#define SHADOW_PCF_NUM_SAMPLES 30 // 7

float3 MaterialDiffuse : register(c0);
float3 LightDirection : register(c1);
float4 LightDiffuse : register(c2);
float4 AmbientColor : register(c3);
float3 CameraPos : register(c4);
float4 FogColor : register(c5);
float2 FogRange : register(c6);
float3 MaterialSpecular : register(c7);
float Shininess : register(c8);
float3 LightSpecular : register(c9);
int ShadowOpacity : register(c10); // 0..100?
float SpecularLight : register(c11);
float specularPower : register(c15);
float kingdomSelector : register(c88);
float isNight : register(c89);

sampler2D DiffuseTexture : register(s0);
sampler2D ShadowMap1 : register(s2); // big shadowmap
sampler2D ShadowMap2 : register(s3); // local (small) shadowmap

static const float PI = 3.1416f;
static const float2 pcfOffsets[30] = {
    float2(0.0000, 0.0000),
    float2(0.5000, 0.0000),
    float2(0.3536, 0.3536),
    float2(0.0000, 0.5000),
    float2(-0.3536, 0.3536),
    float2(-0.5000, 0.0000),
    float2(-0.3536, -0.3536),
    float2(-0.0000, -0.5000),
    float2(0.3536, -0.3536),
    float2(1.0000, 0.0000),
    float2(0.8660, 0.5000),
    float2(0.5000, 0.8660),
    float2(0.0000, 1.0000),
    float2(-0.5000, 0.8660),
    float2(-0.8660, 0.5000),
    float2(-1.0000, 0.0000),
    float2(-0.8660, -0.5000),
    float2(-0.5000, -0.8660),
    float2(-0.0000, -1.0000),
    float2(0.5000, -0.8660),
    float2(0.8660, -0.5000),
    float2(1.5000, 0.0000),
    float2(1.1491, 0.9642),
    float2(0.2605, 1.4772),
    float2(-0.7500, 1.2990),
    float2(-1.4095, 0.5130),
    float2(-1.4095, -0.5130),
    float2(-0.7500, -1.2990),
    float2(0.2605, -1.4772),
    float2(1.1491, -0.9642)
};

float2 clipSpaceToTextureSpace(float4 clipSpace)
{
    float2 cs = clipSpace.xy;
    return float2(0.5f * cs.x, -0.5f * cs.y) + 0.5f;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 Color = tex2D(DiffuseTexture, input.UV);

    float2 texelSize = 1.0f / float2(512.0f, 512.0f);

    float3 tl = tex2D(DiffuseTexture, input.UV + float2(-texelSize.x, -texelSize.y)).rgb;
    float3 t = tex2D(DiffuseTexture, input.UV + float2(0, -texelSize.y)).rgb;
    float3 tr = tex2D(DiffuseTexture, input.UV + float2(texelSize.x, -texelSize.y)).rgb;
    float3 r = tex2D(DiffuseTexture, input.UV + float2(texelSize.x, 0)).rgb;
    float3 br = tex2D(DiffuseTexture, input.UV + float2(texelSize.x, texelSize.y)).rgb;
    float3 b = tex2D(DiffuseTexture, input.UV + float2(0, texelSize.y)).rgb;
    float3 bl = tex2D(DiffuseTexture, input.UV + float2(-texelSize.x, texelSize.y)).rgb;
    float3 l = tex2D(DiffuseTexture, input.UV + float2(-texelSize.x, 0)).rgb;

    float3 dx = tr + 2.0f * r + br - (tl + 2.0f * l + bl);
    float3 dy = bl + 2.0f * b + br - (tl + 2.0f * t + tr);

    float3 NM = normalize(float3(dx.r - dy.r, dx.g - dy.g, 0.048f));
    NM = NM * 0.5 + 0.5;

    float3 N = normalize(input.Normal * NM);
    float3 L = -LightDirection;

    float LightIntensity = saturate(dot(N, L));
    float3 LightResult = (AmbientColor.rgb + (MaterialDiffuse * LightDiffuse.rgb) * LightIntensity * 0.6f);

#ifdef USE_SHADOWS
    const float ShadowBias = 0.0002f;
    const float ShadowBias2 = 0.00004f;
    const float2 LightTexCoord1 = clipSpaceToTextureSpace(input.LightViewPosition1);
    const float2 LightTexCoord2 = clipSpaceToTextureSpace(input.LightViewPosition2);
    const float fCurrDepth1 = input.LightViewPosition1.z - ShadowBias;
    const float fCurrDepth2 = input.LightViewPosition2.z - ShadowBias2;
	 
    float bigShadow = 0.0f;
    float localShadow = 0.0f;
    const float texelScale1 = 1.0f / 1024.0f;
    const float texelScale2 = 1.0f / 1024.0f;
    const float increment = 1.0f / SHADOW_PCF_NUM_SAMPLES;
   
    for (int i = 0; i < SHADOW_PCF_NUM_SAMPLES; ++i)
    {
        float2 offset = pcfOffsets[i];
        bigShadow += (fCurrDepth1 > tex2D(ShadowMap1, LightTexCoord1 + offset * texelScale1).r ? increment : 0.0f);
        localShadow += (fCurrDepth2 > tex2D(ShadowMap2, LightTexCoord2 + offset * texelScale2).r ? increment : 0.0f);
    }

    float borderDist = 0.5f - max(abs(0.5f - LightTexCoord2.x), abs(0.5f - LightTexCoord2.y));
    float fade = smoothstep(0.0f, 0.2f, borderDist);
	 
    float finalShadow = lerp(bigShadow, localShadow, fade);
    float InvertedOpacity = 1.0f - (65.0f / 120.0f);

    finalShadow = lerp(bigShadow, localShadow, fade); 
    finalShadow *= InvertedOpacity;

    LightResult *= (1.0f - finalShadow);

    float3 V = normalize(CameraPos - input.WorldPos.xyz);
    float3 R = reflect(-L, N);
    float SpecularIntensity = Color.b * pow(saturate(dot(V, R)), Shininess * 0.005f);

    float shadowFactor = saturate(smoothstep(0.1f, 0.5f, finalShadow));
    SpecularIntensity *= lerp(5.5f, 0.1f, shadowFactor);
    float distanceFactor = 0.65f;    

    float3 lightResultColor = SpecularIntensity * distanceFactor * MaterialSpecular * LightSpecular.rgb;
    LightResult += lightResultColor;    
#endif

    Color.rgb *= LightResult;

    // Fog
    const float fogStart = FogRange.x;
    const float fogEnd = FogRange.y;
    float distance = length(input.WorldPos.xyz - CameraPos);
    float fogFactor = saturate((distance - fogStart) / (fogEnd - fogStart));
    return lerp(Color, FogColor, fogFactor);
}
)";

// ============================================================================
// MODE 4: Upgraded 12-Sample Crisp PCF & 4-Tap Cross Gradient Object Shader
// ============================================================================
const char* shadowmapObjectsPS = R"(
struct PS_INPUT
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float4 LightViewPosition1 : TEXCOORD2;
    float4 LightViewPosition2 : TEXCOORD3;
    float4 WorldPos : TEXCOORD4;
    float3 Tangent : TEXCOORD5;
};

#define USE_SHADOWS
#define SHADOW_PCF_RADIUS 1
#define SHADOW_PCF_NUM_SAMPLES 12

float3 MaterialDiffuse : register(c0);
float3 LightDirection : register(c1);
float4 LightDiffuse : register(c2);
float4 AmbientColor : register(c3);
float3 CameraPos : register(c4);
float4 FogColor : register(c5);
float2 FogRange : register(c6);
float3 MaterialSpecular : register(c7);
float Shininess : register(c8);
float3 LightSpecular : register(c9);
int ShadowOpacity : register(c10); // 0..100?
float SpecularLight : register(c11);
float specularPower : register(c15);
float kingdomSelector : register(c88);
float isNight : register(c89);

sampler2D DiffuseTexture : register(s0);
sampler2D ShadowMap1 : register(s2); // big shadowmap
sampler2D ShadowMap2 : register(s3); // local (small) shadowmap

static const float PI = 3.1416f;
static const float2 pcfOffsets[12] = {
    float2(0.0000, 0.0000),
    float2(0.5000, 0.0000),
    float2(0.3536, 0.3536),
    float2(0.0000, 0.5000),
    float2(-0.3536, 0.3536),
    float2(-0.5000, 0.0000),
    float2(-0.3536, -0.3536),
    float2(-0.0000, -0.5000),
    float2(0.3536, -0.3536),
    float2(0.7500, 0.0000),
    float2(-0.3750, 0.6495),
    float2(-0.3750, -0.6495)
};

float2 clipSpaceToTextureSpace(float4 clipSpace)
{
    float2 cs = clipSpace.xy;
    return float2(0.5f * cs.x, -0.5f * cs.y) + 0.5f;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 Color = tex2D(DiffuseTexture, input.UV);

    // 4-tap cross gradient (44% fewer texture lookups than 9-tap Sobel with identical normal relief)
    float2 texelSize = 1.0f / float2(512.0f, 512.0f);
    float3 t = tex2D(DiffuseTexture, input.UV + float2(0, -texelSize.y)).rgb;
    float3 r = tex2D(DiffuseTexture, input.UV + float2(texelSize.x, 0)).rgb;
    float3 b = tex2D(DiffuseTexture, input.UV + float2(0, texelSize.y)).rgb;
    float3 l = tex2D(DiffuseTexture, input.UV + float2(-texelSize.x, 0)).rgb;

    float3 dx = (r - l) * 2.0f;
    float3 dy = (b - t) * 2.0f;

    float3 NM = normalize(float3(dx.r - dy.r, dx.g - dy.g, 0.048f));
    NM = NM * 0.5f + 0.5f;

    float3 N = normalize(input.Normal * NM);
    float3 L = -LightDirection;

    float LightIntensity = saturate(dot(N, L));
    float3 LightResult = (AmbientColor.rgb + (MaterialDiffuse * LightDiffuse.rgb) * LightIntensity * 0.6f);

#ifdef USE_SHADOWS
    const float ShadowBias = 0.0002f;
    const float ShadowBias2 = 0.00004f;
    const float2 LightTexCoord1 = clipSpaceToTextureSpace(input.LightViewPosition1);
    const float2 LightTexCoord2 = clipSpaceToTextureSpace(input.LightViewPosition2);
    const float fCurrDepth1 = input.LightViewPosition1.z - ShadowBias;
    const float fCurrDepth2 = input.LightViewPosition2.z - ShadowBias2;
	 
    float bigShadow = 0.0f;
    float localShadow = 0.0f;
    const float texelScale1 = 1.0f / 2048.0f;
    const float texelScale2 = 1.0f / 3072.0f;
    const float increment = 1.0f / SHADOW_PCF_NUM_SAMPLES;
   
    for (int i = 0; i < SHADOW_PCF_NUM_SAMPLES; ++i)
    {
        float2 offset = pcfOffsets[i];
        bigShadow += (fCurrDepth1 > tex2D(ShadowMap1, LightTexCoord1 + offset * texelScale1).r ? increment : 0.0f);
        localShadow += (fCurrDepth2 > tex2D(ShadowMap2, LightTexCoord2 + offset * texelScale2).r ? increment : 0.0f);
    }

    float borderDist = 0.5f - max(abs(0.5f - LightTexCoord2.x), abs(0.5f - LightTexCoord2.y));
    float fade = smoothstep(0.01f, 0.06f, borderDist);
	 
    float finalShadow = lerp(bigShadow, localShadow, fade);
    finalShadow = smoothstep(0.12f, 0.88f, finalShadow);

    float InvertedOpacity = 1.0f - (65.0f / 120.0f);
    finalShadow *= InvertedOpacity;

    LightResult *= (1.0f - finalShadow);

    float3 V = normalize(CameraPos - input.WorldPos.xyz);
    float3 R = reflect(-L, N);
    float SpecularIntensity = Color.b * pow(saturate(dot(V, R)), Shininess * 0.005f);

    float shadowFactor = saturate(smoothstep(0.1f, 0.5f, finalShadow));
    SpecularIntensity *= lerp(5.5f, 0.1f, shadowFactor);
    float distanceFactor = 0.65f;    

    float3 lightResultColor = SpecularIntensity * distanceFactor * MaterialSpecular * LightSpecular.rgb;
    LightResult += lightResultColor;    
#endif

    Color.rgb *= LightResult;

    // Fog
    const float fogStart = FogRange.x;
    const float fogEnd = FogRange.y;
    float distance = length(input.WorldPos.xyz - CameraPos);
    float fogFactor = saturate((distance - fogStart) / (fogEnd - fogStart));
    return lerp(Color, FogColor, fogFactor);
}
)";