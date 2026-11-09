const char* grassVertexShader = R"(
struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float4 LightViewPosition1 : TEXCOORD2; 
    float4 LightViewPosition2 : TEXCOORD3; 
    float4 WorldPos : TEXCOORD4;
};

float4x4 Transform : register(c20);
float4x4 ViewProj : register(c4);
float4x4 LightViewProj1 : register(c8);  
float4x4 LightViewProj2 : register(c16); 
float3 WindDirection : register(c46);    
float WindStrength : register(c47);      
float Time : register(c48);              
float3 GrassPosition : register(c49);   // Character position

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;

    float sway = sin(input.Position.x * 0.1 + Time) * cos(input.Position.z * 0.1 + Time) * WindStrength;
    float3 displacedPosition = input.Position + WindDirection * sway;
    float4 worldPosition = mul(float4(displacedPosition, 1.0f), Transform);

    float3 grassPosToCharacter = worldPosition.xyz - GrassPosition;
    float distanceSquared = dot(grassPosToCharacter.xy, grassPosToCharacter.xy); 

    if (distanceSquared < 150.0f * 150.0f)
    {
        worldPosition = mul(float4(input.Position, 1.0f), Transform);
    }

    if (distanceSquared < 90.0f * 90.0f)
    {
        worldPosition.z -= 25.0f; 
    }

    output.WorldPos = worldPosition;
    output.Position = mul(worldPosition, ViewProj);
    output.Normal = normalize(mul(input.Normal, (float3x3)Transform));
    output.UV = input.UV;
    output.LightViewPosition1 = mul(worldPosition, LightViewProj1);
    output.LightViewPosition2 = mul(worldPosition, LightViewProj2);
    return output;
}
)";
