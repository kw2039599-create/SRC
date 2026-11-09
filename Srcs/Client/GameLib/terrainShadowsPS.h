// ============================================================================
// MODE 3: Original Ground Color + Rain Puddle Reflections ONLY (No Bump/Spec)
// ============================================================================
const char* terrainBasePS_Mode3 = R"(
struct PS_INPUT
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD0;
    float2 UV2 : TEXCOORD1;
    float3 Normal : TEXCOORD2;
    float4 WorldPos : TEXCOORD4;
    float3 Tangent : TEXCOORD5;
    float4 LightViewPosition1 : TEXCOORD6;
    float4 LightViewPosition2 : TEXCOORD7;
    float4 ScreenPos : TEXCOORD8;
    float4 Diffuse : COLOR0;
};

float3 CameraPos : register(c4);
float4 FogColor : register(c5);
float2 FogRange : register(c6);
float time : register(c12);
float alphaOverride : register(c15);
float uv : register(c17);
float3 characterPosition : register(c86);
float puddleThreshold : register(c87);
float3 PrevieousCharacterPosition : register(c93);
float isMoving : register(c94);

sampler2D DiffuseTexture : register(s0);
sampler2D SplatTexture : register(s1);
sampler2D normalMap1 : register(s5);
sampler2D normalMap2 : register(s6);
sampler2D ReflectionTexture : register(s7);

static const float PI = 3.1416f;

float random(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

float smoothNoise(float2 uvCoord)
{
    float2 i = floor(uvCoord);
    float2 f = frac(uvCoord);
    f = f * f * (3.0 - 2.0 * f);

    float a = random(i);
    float b = random(i + float2(1.0, 0.0));
    float c = random(i + float2(0.0, 1.0));
    float d = random(i + float2(1.0, 1.0));

    return lerp(lerp(a, b, f.x), lerp(c, d, f.x), f.y);
}

float isWithinPuddle(float2 uvCoord, float3 normal)
{
    if (puddleThreshold >= 1.0f)
        return 0.0f;

    float noiseScale = 1.0f;
    float noiseValue = smoothNoise(uvCoord * noiseScale);

    float spread = lerp(0.4f, 0.02f, saturate(puddleThreshold));
    float edgeAA = fwidth(noiseValue);
    float puddlePresence = smoothstep(puddleThreshold - spread - edgeAA, puddleThreshold + spread + edgeAA, noiseValue);

    float flatThreshold = 1e-10; 
    float flatnessFactor = 0.15;

    float smoothX = smoothstep(flatThreshold, flatThreshold + flatnessFactor, abs(normal.x));
    float smoothY = smoothstep(flatThreshold, flatThreshold + flatnessFactor, abs(normal.y));

    float isFlatSurface = 0.0f;
    if (smoothX < 1.0f && smoothY < 1.0f)
    {
        isFlatSurface = smoothstep(0.0, 1.0, 1.0f - max(smoothX, smoothY));
    }

    return puddlePresence * isFlatSurface;
}

float circularWave(float distance, float frequency, float speed, float amplitude)
{
    return amplitude * sin(distance * frequency - time * speed);
}

float3 interpolate(float3 start, float3 end, float t)
{
    return start + t * (end - start);
}

float4 main(PS_INPUT input) : SV_TARGET
{
    // Sample using exact world-transform UV to match BEFORE fixed-function tiling
    float4 Color = tex2D(DiffuseTexture, input.UV);
    float calculatedAlpha = tex2D(SplatTexture, input.UV2).a;
    Color.a = (alphaOverride > 0.5f) ? 1.0f : calculatedAlpha;

    float2 puddleUV = input.UV * uv;
    float isPuddle = isWithinPuddle(puddleUV, input.Normal);

    if (isPuddle > 0.001f)
    {
        float2 reflection = 0.5f * (1.0f - input.ScreenPos.xy);

        float2 foamUV;
        foamUV.x = 0.01f + 0.98f * (input.WorldPos.x * 0.001f);
        foamUV.y = 0.01f + 0.98f * (input.WorldPos.y * 0.001f);

        float2 timeOffset1 = float2(-time * 0.3f * 0.0001975f, 0.0f);
        float2 animatedUV1 = foamUV + timeOffset1;

        float2 timeOffset2 = float2(time * 0.3f * 0.0001975f, 0.0f);
        float2 animatedUV2 = foamUV + timeOffset2;

        float3 normalMapSample1 = tex2Dlod(normalMap1, float4(animatedUV1, 0.0f, 0.0f)).rgb;
        float3 normalMapSample2 = tex2Dlod(normalMap2, float4(animatedUV2, 0.0f, 0.0f)).rgb;

        float3 blendedNormal = normalize((normalMapSample1 * 2.0f - 1.0f) + (normalMapSample2 * 2.0f - 1.0f));

        float perturbationScale = 0.025f;
        float2 perturbedUV = reflection + (blendedNormal.xy * 0.5f) * perturbationScale;

        float3 waveStartPosition = PrevieousCharacterPosition;
        float3 waveEndPosition = characterPosition;
        float3 currentPosition = input.WorldPos.xyz;

        float distanceAlongPath = max(0.001f, length(waveEndPosition - waveStartPosition));
        float t = saturate(dot(currentPosition - waveStartPosition, waveEndPosition - waveStartPosition) / (distanceAlongPath * distanceAlongPath));
        float3 interpolatedPosition = interpolate(waveStartPosition, waveEndPosition, t);

        float distanceFromPath = distance(currentPosition, interpolatedPosition);

        float maxRange = 80.0f;
        float maxRange2 = 300.0f;
        float inRangeFactor1 = saturate(1.0f - (distanceFromPath / maxRange));
        float inRangeFactor2 = saturate(1.0f - (distanceFromPath / maxRange2));

        float waveSpeed = 0.02f;
        float waveFrequency = 0.2f * PI / 1.7f;
        float waveAmplitude = 0.02f * inRangeFactor1;
        float waveAmplitude2 = 0.04f * inRangeFactor2;
        float waveEffect = 0.0f;

        if (isMoving > 0.1f)
            waveEffect = circularWave(distanceFromPath, waveFrequency / 3.0f, waveSpeed / 1.5f, waveAmplitude2);
        else
            waveEffect = circularWave(distanceFromPath / 1.2f, waveFrequency / 2.0f, waveSpeed / 3.0f, waveAmplitude);

        float reflectionInfluence = saturate(1.0f - (distanceFromPath / 20000.0f));

        float4 reflectionColor = tex2Dlod(ReflectionTexture, float4(perturbedUV + waveEffect, 0.0f, 0.0f));
        Color.rgb = lerp(Color.rgb, reflectionColor.rgb * 1.2f, isPuddle * reflectionInfluence);
    }

    float4 finalColor = Color;
    float fogFactor = saturate((length(CameraPos - input.WorldPos.xyz) - FogRange.x) / (FogRange.y - FogRange.x));
    finalColor = lerp(finalColor, FogColor, fogFactor);
    finalColor.a = Color.a;

    return finalColor;
}
)";

// ============================================================================
// MODE 4: High-Quality Ground (2K/4K Normal Maps, Specular Maps & Puddles)
// ============================================================================
const char* terrainBasePS = R"(
struct PS_INPUT
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD0;
    float2 UV2 : TEXCOORD1;
    float3 Normal : TEXCOORD2;
    float4 WorldPos : TEXCOORD4;
    float3 Tangent : TEXCOORD5;
    float4 LightViewPosition1 : TEXCOORD6;
    float4 LightViewPosition2 : TEXCOORD7;
    float4 ScreenPos : TEXCOORD8;
    float4 Diffuse : COLOR0;
};

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
float textureDiffusePower : register(c10);
float SpecularLight : register(c11);
float time : register(c12);
float terrainLightMultiply : register(c14);
float alphaOverride : register(c15);
float isSnow : register(c16);
float uv : register(c17);
float textureSpecularPower : register(c18);
float3 characterPosition : register(c86);
float puddleThreshold : register(c87);
float kingdomSelector : register(c88);
float isNight : register(c89);
float isStorm : register(c90);
float3 PrevieousCharacterPosition : register(c93);
float isMoving : register(c94);

sampler2D DiffuseTexture : register(s0);
sampler2D SplatTexture : register(s1);
sampler2D NormalMap : register(s2);
sampler2D shadowMap1 : register(s3);
sampler2D SpecularMap : register(s4);
sampler2D normalMap1 : register(s5);
sampler2D normalMap2 : register(s6);
sampler2D ReflectionTexture : register(s7);
sampler2D foamTexture : register(s8);
sampler2D AttrTexture : register(s9);
sampler2D AttrTexture2 : register(s10);

static const float PI = 3.1416f;

float2 clipSpaceToTextureSpace(float4 clipSpace)
{
    float2 cs = clipSpace.xy;
    return float2(0.5f * cs.x, -0.5f * cs.y) + 0.5f;
}

float random(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

float smoothNoise(float2 uv)
{
    float2 i = floor(uv);
    float2 f = frac(uv);
    f = f * f * (3.0 - 2.0 * f);

    float a = random(i);
    float b = random(i + float2(1.0, 0.0));
    float c = random(i + float2(0.0, 1.0));
    float d = random(i + float2(1.0, 1.0));

    return lerp(lerp(a, b, f.x), lerp(c, d, f.x), f.y);
}

float isWithinPuddle(float2 uv, float3 normal)
{
    if (puddleThreshold >= 1.0f)
        return 0.0f;

    float noiseScale = 1.0f;
    float noiseValue = smoothNoise(uv * noiseScale);

    float spread = lerp(0.4f, 0.02f, saturate(puddleThreshold));
    float edgeAA = fwidth(noiseValue);
    float puddlePresence = smoothstep(puddleThreshold - spread - edgeAA, puddleThreshold + spread + edgeAA, noiseValue);

    float flatThreshold = 1e-10; 
    float flatnessFactor = 0.15;

    float smoothX = smoothstep(flatThreshold, flatThreshold + flatnessFactor, abs(normal.x));
    float smoothY = smoothstep(flatThreshold, flatThreshold + flatnessFactor, abs(normal.y));

    float isFlatSurface = 0.0f;
    if (smoothX < 1.0f && smoothY < 1.0f)
    {
        isFlatSurface = smoothstep(0.0, 1.0, 1.0f - max(smoothX, smoothY));
    }

    return puddlePresence * isFlatSurface;
}

float circularWave(float distance, float frequency, float speed, float amplitude)
{
    return amplitude * sin(distance * frequency - time * speed);
}

float3 interpolate(float3 start, float3 end, float t)
{
    return start + t * (end - start);
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float2 tiledUV = input.UV / uv;
    float4 Color = tex2D(DiffuseTexture, tiledUV);
    float calculatedAlpha = tex2D(SplatTexture, input.UV2).a;
    Color.a = (alphaOverride > 0.5f) ? 1.0f : saturate(calculatedAlpha * 1.5f);

    float3 rawNM = (tex2D(NormalMap, tiledUV).rgb * 2.0f) - 1.0f;
    // Screen-space derivative filtering on normal map to prevent terrain shimmer at a distance
    float2 uvFw = fwidth(tiledUV);
    float normalMipFade = saturate(1.0f / (1.0f + max(uvFw.x, uvFw.y) * 16.0f));
    float3 NM = normalize(lerp(float3(0.0f, 0.0f, 1.0f), rawNM, normalMipFade));

    float3 L = normalize(-LightDirection);
    float diffuseLight = saturate(dot(NM, L));

    const float2 LightTexCoord1 = clipSpaceToTextureSpace(input.LightViewPosition2);
    const float fCurrDepth1 = input.LightViewPosition2.z - 0.0001f;

    // Specular Anti-Aliasing (prevents Shininess * 100 from sparkling)
    float3 dNMdx = ddx(NM);
    float3 dNMdy = ddy(NM);
    float normalVariance = dot(dNMdx, dNMdx) + dot(dNMdy, dNMdy);
    float specAA = 1.0f / (1.0f + normalVariance * 25.0f);

    float3 V = normalize(CameraPos - input.WorldPos.xyz);
    float3 R = reflect(-L, NM);
    float specularComponent = pow(saturate(dot(V, R)), max(1.0f, Shininess * 100.0f * specAA)) * 10.5f * specAA;

    float3 specularLight = float3(0, 0, 0);
    float3 specularColor = tex2D(SpecularMap, tiledUV).r;
    if (fCurrDepth1 < tex2D(shadowMap1, LightTexCoord1).r)
        specularLight = specularComponent * (specularColor * textureSpecularPower);
    else
        specularLight = specularComponent * (specularColor * (textureSpecularPower / 3.0f));

    float isPuddle = isWithinPuddle(input.UV, input.Normal);
    float puddleAA = max(fwidth(isPuddle), 0.005f);
    float transition = smoothstep(0.01f - puddleAA, 0.03f + puddleAA, isPuddle);
    specularLight *= lerp(0.06f, isPuddle, transition);

    float LightIntensity = 0.85f + 0.25f * diffuseLight + 0.02f * (specularLight / 2.5f);
    Color.rgb *= LightIntensity;

    if (isPuddle > 0.001f)
    {
        float2 reflection = 0.5f * (1.0f - input.ScreenPos.xy);

        float2 foamUV;
        foamUV.x = 0.01f + 0.98f * (input.WorldPos.x * 0.001f);
        foamUV.y = 0.01f + 0.98f * (input.WorldPos.y * 0.001f);

        float2 timeOffset1 = float2(-time * 0.3f * 0.0001975f, 0.0f);
        float2 animatedUV1 = foamUV + timeOffset1;

        float2 timeOffset2 = float2(time * 0.3f * 0.0001975f, 0.0f);
        float2 animatedUV2 = foamUV + timeOffset2;

        float3 normalMapSample1 = tex2Dlod(normalMap1, float4(animatedUV1, 0.0f, 0.0f)).rgb;
        float3 normalMapSample2 = tex2Dlod(normalMap2, float4(animatedUV2, 0.0f, 0.0f)).rgb;

        float3 blendedNormal = normalize((normalMapSample1 * 2.0f - 1.0f) + (normalMapSample2 * 2.0f - 1.0f));

        float perturbationScale = 0.025f;
        float2 perturbedUV = reflection + (blendedNormal.xy * 0.5f) * perturbationScale;

        float3 waveStartPosition = PrevieousCharacterPosition;
        float3 waveEndPosition = characterPosition;
        float3 currentPosition = input.WorldPos.xyz;

        float distanceAlongPath = max(0.001f, length(waveEndPosition - waveStartPosition));
        float t = saturate(dot(currentPosition - waveStartPosition, waveEndPosition - waveStartPosition) / (distanceAlongPath * distanceAlongPath));
        float3 interpolatedPosition = interpolate(waveStartPosition, waveEndPosition, t);

        float distanceFromPath = distance(currentPosition, interpolatedPosition);

        float maxRange = 80.0f;
        float maxRange2 = 300.0f;
        float inRangeFactor1 = saturate(1.0f - (distanceFromPath / maxRange));
        float inRangeFactor2 = saturate(1.0f - (distanceFromPath / maxRange2));

        float waveSpeed = 0.02f;
        float waveFrequency = 0.2f * PI / 1.7f;
        float waveAmplitude = 0.02f * inRangeFactor1;
        float waveAmplitude2 = 0.04f * inRangeFactor2;
        float waveEffect = 0.0f;

        if (isMoving > 0.1f)
            waveEffect = circularWave(distanceFromPath, waveFrequency / 3.0f, waveSpeed / 1.5f, waveAmplitude2);
        else
            waveEffect = circularWave(distanceFromPath / 1.2f, waveFrequency / 2.0f, waveSpeed / 3.0f, waveAmplitude);

        float reflectionInfluence = saturate(1.0f - (distanceFromPath / 20000.0f));

        float4 reflectionColor = tex2Dlod(ReflectionTexture, float4(perturbedUV + waveEffect, 0.0f, 0.0f));
        Color.rgb = lerp(Color.rgb, reflectionColor.rgb * 1.2f, isPuddle * reflectionInfluence);
    }

    float4 finalColor = Color;
    float fogFactor = saturate((length(CameraPos - input.WorldPos.xyz) - FogRange.x) / (FogRange.y - FogRange.x));
    finalColor = lerp(finalColor, FogColor, fogFactor);
    finalColor.a = Color.a;

    return finalColor;
}
)";

// ============================================================================
// MODE 3 SHADOW PASS: Restored 30-Sample PCF Terrain Shadows (from BEFORE)
// ============================================================================
const char* terrainShadowsPS_Mode3 = R"(
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
float Time : register(c12);
float SpecularLight : register(c11);
float terrainLightMultiply : register(c14);

sampler2D DiffuseTexture : register(s0);
sampler2D ShadowMap1 : register(s1);   // big shadowmap
sampler2D ShadowMap2 : register(s2);   // local (small) shadowmap

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

    // Fetch and transform normal from the diffuse map (pretending it's a normal map)
    float3 NM = normalize((tex2D(DiffuseTexture, input.UV).rgb * 2.0f) - 1.0f);
    float3 N = normalize(input.Normal * NM);
    float3 L = -LightDirection;

    float OriginalLightIntensity = saturate(dot(N, L));
    float LightIntensity = 0.3f + 0.2f * OriginalLightIntensity;
    float3 LightResult = (AmbientColor.rgb + (MaterialDiffuse * LightDiffuse.rgb) * LightIntensity * 0.6f);

#ifdef USE_SHADOWS
    const float ShadowBias = 0.0005f;
    const float ShadowBias2 = 0.00004f;
    const float2 LightTexCoord1 = clipSpaceToTextureSpace(input.LightViewPosition1);
    const float2 LightTexCoord2 = clipSpaceToTextureSpace(input.LightViewPosition2);
    const float fCurrDepth1 = input.LightViewPosition1.z - ShadowBias;
    const float fCurrDepth2 = input.LightViewPosition2.z - ShadowBias2;
	 
    float bigShadow = 0.0f;
    float localShadow = 0.0f;	 
    const float texelScale1 = 1.0f / 512.0f;
    const float texelScale2 = 1.0f / 2048.0f;
    const float increment = 1.0f / SHADOW_PCF_NUM_SAMPLES;
   
    for (int i = 0; i < SHADOW_PCF_NUM_SAMPLES; ++i) {		 		
        float2 offset = pcfOffsets[i];				
        bigShadow   += (fCurrDepth1 > tex2D(ShadowMap1, LightTexCoord1 + offset * texelScale1).r ? increment : 0.0f);
        localShadow += (fCurrDepth2 > tex2D(ShadowMap2, LightTexCoord2 + offset * texelScale2).r ? increment : 0.0f);      
    }

    // Distance to the inner shadow border (0.0 .. 0.5)
    float borderDist = 0.5f - max(abs(0.5f - LightTexCoord2.x), abs(0.5f - LightTexCoord2.y));
    float fade = smoothstep(0.0f, 0.2f, borderDist);

    float finalShadow = lerp(bigShadow, localShadow, fade);
    float InvertedOpacity = 1.0f - (ShadowOpacity / 120.0f);

    finalShadow = lerp(bigShadow, localShadow, fade); 
    finalShadow *= InvertedOpacity;

    LightResult *= (1.0f - finalShadow);

    float shadowThreshold = 0.3f;
    float smoothShadowFactor = smoothstep(shadowThreshold, shadowThreshold - 0.1f, finalShadow);
    
    if (smoothShadowFactor > 0.0f)
    {
        float3 V = normalize(CameraPos - input.WorldPos.xyz);
        float3 R = reflect(-L, N);
        float SpecularIntensity = pow(saturate(dot(V, R)), Shininess) / 1.5f;
        SpecularIntensity *= smoothShadowFactor;
        LightResult += (SpecularIntensity * MaterialSpecular * LightSpecular.rgb) / 1.5f;
    }
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
// MODE 4 SHADOW PASS: Crisp Bilinear Terrain Shadows & Mip-Attenuated Normals
// ============================================================================
const char* terrainShadowsPS = R"(
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
float Time : register(c12);
float SpecularLight : register(c11);
float terrainLightMultiply : register(c14);

sampler2D DiffuseTexture : register(s0);
sampler2D ShadowMap1 : register(s1);   // big shadowmap
sampler2D ShadowMap2 : register(s2);   // local (small) shadowmap

float2 clipSpaceToTextureSpace(float4 clipSpace)
{
    float2 cs = clipSpace.xy;
    return float2(0.5f * cs.x, -0.5f * cs.y) + 0.5f;
}

// Emulates hardware bilinear filtering on R32F depth maps using tex2Dlod for branch-safe cascade culling
float SampleCrispShadow(sampler2D shadowMap, float2 uv, float compareDepth)
{
    const float mapSize = 2048.0f;
    const float texelSize = 1.0f / 2048.0f;

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

    float3 rawNM = (Color.rgb * 2.0f) - 1.0f;
    float2 uvFw = fwidth(input.UV);
    float mipFade = saturate(1.0f / (1.0f + max(uvFw.x, uvFw.y) * 16.0f));
    float3 NM = normalize(lerp(float3(0.0f, 0.0f, 1.0f), rawNM, mipFade));

    float3 N = normalize(input.Normal * NM);
    float3 L = -LightDirection;

    float OriginalLightIntensity = saturate(dot(N, L));
    float LightIntensity = 0.3f + 0.2f * OriginalLightIntensity;
    float3 LightResult = (AmbientColor.rgb + (MaterialDiffuse * LightDiffuse.rgb) * LightIntensity * 0.6f);

#ifdef USE_SHADOWS
    const float ShadowBias = 0.00035f;
    const float ShadowBias2 = 0.00005f;
    const float2 LightTexCoord1 = clipSpaceToTextureSpace(input.LightViewPosition1);
    const float2 LightTexCoord2 = clipSpaceToTextureSpace(input.LightViewPosition2);
    const float fCurrDepth1 = input.LightViewPosition1.z - ShadowBias;
    const float fCurrDepth2 = input.LightViewPosition2.z - ShadowBias2;

    float borderDist = 0.5f - max(abs(0.5f - LightTexCoord2.x), abs(0.5f - LightTexCoord2.y));
    float fade = smoothstep(0.0f, 0.12f, borderDist);

    float bigShadow = 0.0f;
    float localShadow = 0.0f;

    if (fade < 0.999f)
        bigShadow = SampleCrispShadow(ShadowMap1, LightTexCoord1, fCurrDepth1);
    if (fade > 0.001f)
        localShadow = SampleCrispShadow(ShadowMap2, LightTexCoord2, fCurrDepth2);

    float finalShadow = lerp(bigShadow, localShadow, fade);
    float InvertedOpacity = 1.0f - (ShadowOpacity / 120.0f);
    finalShadow *= InvertedOpacity;

    LightResult *= (1.0f - finalShadow);

    float shadowThreshold = 0.3f;
    float smoothShadowFactor = smoothstep(shadowThreshold, shadowThreshold - 0.1f, finalShadow);
    
    if (smoothShadowFactor > 0.0f)
    {
        float3 dNdx = ddx(N);
        float3 dNdy = ddy(N);
        float normalVariance = dot(dNdx, dNdx) + dot(dNdy, dNdy);
        float specAA = 1.0f / (1.0f + normalVariance * 12.0f);

        float3 V = normalize(CameraPos - input.WorldPos.xyz);
        float3 R = reflect(-L, N);
        float SpecularIntensity = pow(saturate(dot(V, R)), max(1.0f, Shininess * specAA)) * (specAA / 1.5f);
        SpecularIntensity *= smoothShadowFactor;
        LightResult += (SpecularIntensity * MaterialSpecular * LightSpecular.rgb) / 1.5f;
    }
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