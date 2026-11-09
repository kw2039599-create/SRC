// Mode 3: Original 30-sample PCF shadow & procedural normal map (from shadowmapPSBEFORE.h)
const char* shadowmapPS_Mode3 = R"(
struct PS_INPUT
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float4 LightViewPosition1 : TEXCOORD2;
    float4 LightViewPosition2 : TEXCOORD3;
    float4 LightViewSpecular : TEXCOORD4;
    float4 WorldPos : TEXCOORD5;
    float3 Tangent : TEXCOORD6;
};

#define USE_SHADOWS
#define SHADOW_PCF_RADIUS 2
#define SHADOW_PCF_NUM_SAMPLES 30

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
float Timer : register(c13);
float SpecularPowerMaterial : register(c14);
float specularPower : register(c15);
float blendTransition : register(c83); 
float kingdomSelector : register(c88);
float isNight : register(c89);
float isStorm : register(c90);
float3 SpecularTrans : register(c91);
float SpecularPower : register(c92); 

sampler2D DiffuseTexture : register(s0);
sampler2D SpecularTexture : register(s1);
sampler2D ShadowMap1 : register(s2); // big shadowmap
sampler2D ShadowMap2 : register(s3); // local (small) shadowmap
sampler2D NormalMap : register(s4);

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

float ComputePointLightIntensity(float3 position, float3 lightPosition, float radius)
{
    float3 toLightPosition = position - lightPosition;
    float distanceToLight = length(toLightPosition);
    return pow(saturate(1.0f - distanceToLight / radius), 2);
}

float2 clipSpaceToTextureSpace(float4 clipSpace)
{
    float2 cs = clipSpace.xy;
    return float2(0.5f * cs.x, -0.5f * cs.y) + 0.5f;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 Color = tex2D(DiffuseTexture, input.UV);

    // Fetch and transform normal from normal map
    float2 texelSize = 1.0f / float2(2048.0f, 2048.0f);
    float3 left = tex2D(DiffuseTexture, input.UV + float2(-texelSize.x, 0)).rgb;
    float3 right = tex2D(DiffuseTexture, input.UV + float2(texelSize.x, 0)).rgb;
    float3 top = tex2D(DiffuseTexture, input.UV + float2(0, -texelSize.y)).rgb;
    float3 bottom = tex2D(DiffuseTexture, input.UV + float2(0, texelSize.y)).rgb;

    float3 dx = right - left;
    float3 dy = bottom - top;

    float3 NM;

    NM = normalize(float3(dy.r - dx.r, dy.g - dx.g, 0.05f));

    NM = NM * 0.5 + 0.5; // Adjust to [0, 1] range

    float3 N = normalize(input.Normal * NM);
    float3 L = -LightDirection;

    float LightIntensity = saturate(dot(N, L));
    float3 LightResult = (AmbientColor.rgb + (MaterialDiffuse * LightDiffuse.rgb) * LightIntensity * 0.7f);

#ifdef USE_SHADOWS
    const float ShadowBias = 0.000025f;
    const float2 LightTexCoord1 = clipSpaceToTextureSpace(input.LightViewPosition1);
    const float2 LightTexCoord2 = clipSpaceToTextureSpace(input.LightViewPosition2);
    const float fCurrDepth1 = input.LightViewPosition1.z - ShadowBias;
    const float fCurrDepth2 = input.LightViewPosition2.z - ShadowBias;
	 
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

    float specularShadowFactor = 1.0f - finalShadow;
#endif

    if (Color.a < 1.0 && Color.a > 0.0f)
    {
        float3 V = normalize(CameraPos - input.WorldPos.xyz);
        float3 H = normalize(L + V);
        float3 reflectionVector = reflect(-V, N);
        float2 SpecularUV = reflectionVector.xy * 0.5 + 0.5f;
        SpecularUV += SpecularTrans.xy * 0.5;
        float4 SpecularTex = tex2D(SpecularTexture, SpecularUV);
        float specularIntensity = saturate(dot(N, V));
        float specularFactor = (pow(specularIntensity, 0.1f) * SpecularTex.a) * SpecularPower;

        float3 SpecularColor = SpecularTex.rgb * specularFactor * 3.0f;
        Color.rgb += SpecularColor * Color.a;
    }

    Color.rgb *= LightResult;

    const float fogStart = FogRange.x;
    const float fogEnd = FogRange.y;
    float distance = length(input.WorldPos.xyz - CameraPos);
    float fogFactor = saturate((distance - fogStart) / (fogEnd - fogStart));
    float4 FinalColor = lerp(Color, FogColor, fogFactor);
    FinalColor.a = Color.a;
    return FinalColor;
}
)";

// Mode 4: Upgraded Crisp Shadow, Mip-Attenuated Normal Detail & Specular Anti-Aliasing
const char* shadowmapPS = R"(
struct PS_INPUT
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float4 LightViewPosition1 : TEXCOORD2;
    float4 LightViewPosition2 : TEXCOORD3;
    float4 LightViewSpecular : TEXCOORD4;
    float4 WorldPos : TEXCOORD5;
    float3 Tangent : TEXCOORD6;
};

#define USE_SHADOWS

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
float Timer : register(c13);
float SpecularPowerMaterial : register(c14);
float specularPower : register(c15);
float blendTransition : register(c83); 
float kingdomSelector : register(c88);
float isNight : register(c89);
float isStorm : register(c90);
float3 SpecularTrans : register(c91);
float SpecularPower : register(c92); 

sampler2D DiffuseTexture : register(s0);
sampler2D SpecularTexture : register(s1);
sampler2D ShadowMap1 : register(s2); // big shadowmap
sampler2D ShadowMap2 : register(s3); // local (small) shadowmap
sampler2D NormalMap : register(s4);

static const float PI = 3.1416f;

float ComputePointLightIntensity(float3 position, float3 lightPosition, float radius)
{
    float3 toLightPosition = position - lightPosition;
    float distanceToLight = length(toLightPosition);
    return pow(saturate(1.0f - distanceToLight / radius), 2);
}

float2 clipSpaceToTextureSpace(float4 clipSpace)
{
    float2 cs = clipSpace.xy;
    return float2(0.5f * cs.x, -0.5f * cs.y) + 0.5f;
}

float SampleCrispShadow(sampler2D shadowMap, float2 uv, float compareDepth, float mapSize)
{
    float texelSize = 1.0f / mapSize;
    float2 texelPos = uv * mapSize - 0.5f;
    float2 f = frac(texelPos);
    float2 baseUV = (floor(texelPos) + 0.5f) * texelSize;

    float3 wx = float3(1.0f - f.x, 1.0f, f.x) * 0.5f;
    float3 wy = float3(1.0f - f.y, 1.0f, f.y) * 0.5f;

    float s00 = (compareDepth > tex2Dlod(shadowMap, float4(baseUV + float2(-texelSize, -texelSize), 0.0f, 0.0f)).r) ? 1.0f : 0.0f;
    float s10 = (compareDepth > tex2Dlod(shadowMap, float4(baseUV + float2( 0.0f,      -texelSize), 0.0f, 0.0f)).r) ? 1.0f : 0.0f;
    float s20 = (compareDepth > tex2Dlod(shadowMap, float4(baseUV + float2( texelSize, -texelSize), 0.0f, 0.0f)).r) ? 1.0f : 0.0f;

    float s01 = (compareDepth > tex2Dlod(shadowMap, float4(baseUV + float2(-texelSize,  0.0f), 0.0f, 0.0f)).r) ? 1.0f : 0.0f;
    float s11 = (compareDepth > tex2Dlod(shadowMap, float4(baseUV,                             0.0f, 0.0f)).r) ? 1.0f : 0.0f;
    float s21 = (compareDepth > tex2Dlod(shadowMap, float4(baseUV + float2( texelSize,  0.0f), 0.0f, 0.0f)).r) ? 1.0f : 0.0f;

    float s02 = (compareDepth > tex2Dlod(shadowMap, float4(baseUV + float2(-texelSize,  texelSize), 0.0f, 0.0f)).r) ? 1.0f : 0.0f;
    float s12 = (compareDepth > tex2Dlod(shadowMap, float4(baseUV + float2( 0.0f,       texelSize), 0.0f, 0.0f)).r) ? 1.0f : 0.0f;
    float s22 = (compareDepth > tex2Dlod(shadowMap, float4(baseUV + float2( texelSize,  texelSize), 0.0f, 0.0f)).r) ? 1.0f : 0.0f;

    float row0 = dot(float3(s00, s10, s20), wx);
    float row1 = dot(float3(s01, s11, s21), wx);
    float row2 = dot(float3(s02, s12, s22), wx);

    return dot(float3(row0, row1, row2), wy);
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 Color = tex2D(DiffuseTexture, input.UV);

    // Attenuate procedural normal detail by screen-space UV derivative to eliminate distant aliasing
    float2 texelSize = 1.0f / float2(2048.0f, 2048.0f);
    float2 uvFw = fwidth(input.UV) * 2048.0f;
    float mipAttenuation = saturate(1.0f / (1.0f + max(uvFw.x, uvFw.y) * 1.2f));

    float3 left   = tex2D(DiffuseTexture, input.UV + float2(-texelSize.x, 0.0f)).rgb;
    float3 right  = tex2D(DiffuseTexture, input.UV + float2( texelSize.x, 0.0f)).rgb;
    float3 top    = tex2D(DiffuseTexture, input.UV + float2(0.0f, -texelSize.y)).rgb;
    float3 bottom = tex2D(DiffuseTexture, input.UV + float2(0.0f,  texelSize.y)).rgb;

    float3 dx = (right - left) * mipAttenuation;
    float3 dy = (bottom - top) * mipAttenuation;

    float3 NM = normalize(float3(dy.r - dx.r, dy.g - dx.g, 0.05f));
    NM = NM * 0.5f + 0.5f;

    float3 N = normalize(input.Normal * NM);
    float3 L = -LightDirection;

    float LightIntensity = saturate(dot(N, L));
    float3 LightResult = (AmbientColor.rgb + (MaterialDiffuse * LightDiffuse.rgb) * LightIntensity * 0.7f);

#ifdef USE_SHADOWS
    const float ShadowBias = 0.000025f;
    const float2 LightTexCoord1 = clipSpaceToTextureSpace(input.LightViewPosition1);
    const float2 LightTexCoord2 = clipSpaceToTextureSpace(input.LightViewPosition2);
    const float fCurrDepth1 = input.LightViewPosition1.z - ShadowBias;
    const float fCurrDepth2 = input.LightViewPosition2.z - ShadowBias;

    float borderDist = 0.5f - max(abs(0.5f - LightTexCoord2.x), abs(0.5f - LightTexCoord2.y));
    float fade = smoothstep(0.0f, 0.2f, borderDist);

    float bigShadow = 0.0f;
    float localShadow = 0.0f;

    if (fade < 0.999f)
        bigShadow = SampleCrispShadow(ShadowMap1, LightTexCoord1, fCurrDepth1, 1024.0f);
    if (fade > 0.001f)
        localShadow = SampleCrispShadow(ShadowMap2, LightTexCoord2, fCurrDepth2, 1024.0f);

    float InvertedOpacity = 1.0f - (65.0f / 120.0f);
    float finalShadow = lerp(bigShadow, localShadow, fade) * InvertedOpacity;

    LightResult *= (1.0f - finalShadow);
#endif

    if (Color.a < 1.0f && Color.a > 0.0f)
    {
        float3 dNdx = ddx(N);
        float3 dNdy = ddy(N);
        float normalVariance = dot(dNdx, dNdx) + dot(dNdy, dNdy);
        float specAA = 1.0f / (1.0f + normalVariance * 10.0f);

        float3 V = normalize(CameraPos - input.WorldPos.xyz);
        float3 reflectionVector = reflect(-V, N);
        float2 SpecularUV = reflectionVector.xy * 0.5f + 0.5f + SpecularTrans.xy * 0.5f;
        float4 SpecularTex = tex2D(SpecularTexture, SpecularUV);
        float specularIntensity = saturate(dot(N, V));
        float specularFactor = (pow(specularIntensity, 0.1f) * SpecularTex.a) * SpecularPower * specAA;

        float3 SpecularColor = SpecularTex.rgb * specularFactor * 3.0f;
        Color.rgb += SpecularColor * Color.a;
    }

    Color.rgb *= LightResult;

    const float fogStart = FogRange.x;
    const float fogEnd = FogRange.y;
    float distance = length(input.WorldPos.xyz - CameraPos);
    float fogFactor = saturate((distance - fogStart) / (fogEnd - fogStart));
    float4 FinalColor = lerp(Color, FogColor, fogFactor);
    FinalColor.a = Color.a;
    return FinalColor;
}
)";