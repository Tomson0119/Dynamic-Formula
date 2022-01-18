#include "lighting.hlsl"

Texture2D gShadowMap[3] : register(t0, space1);

SamplerState gAnisotropicWrap  : register(s0);
SamplerState gAnisotropicClamp : register(s1);
SamplerState gLinearWrap       : register(s2);
SamplerState gLinearClamp      : register(s3);
SamplerComparisonState gPCFShadow : register(s4);

cbuffer CameraCB : register(b0)
{
    matrix gView            : packoffset(c0);
    matrix gProj            : packoffset(c4);
    matrix gViewProj        : packoffset(c8);
    float3 gCameraPos       : packoffset(c12);
    float  gAspect          : packoffset(c12.w);
}

cbuffer LightCB : register(b1)
{
    matrix gShadowTransform    : packoffset(c0); // need as much as shadow map
    float4 gAmbient            : packoffset(c4);
    Light  gLights[NUM_LIGHTS] : packoffset(c5);
}

cbuffer GameInfoCB : register(b2)
{
    float4 gRandFloat4  : packoffset(c0);
    float3 gPlayerPos   : packoffset(c1);
    uint   gKeyInput    : packoffset(c1.w);
    float  gCurrentTime : packoffset(c2.x);
    float  gElapsedTime : packoffset(c2.y);
}

cbuffer MaterialCB : register(b3)
{
    matrix gTexTransform : packoffset(c0);
    Material gMat        : packoffset(c4);    
}

cbuffer ObjectCB : register(b4)
{
    matrix gWorld : packoffset(c0);
}

cbuffer ShadowCB : register(b5)
{
    float gZSplits[3] : packoffset(c0);
    matrix gShadowViewProj[3] : packoffset(c4);
}

float CalcShadowFactor(float4 shadowPos, int idx)
{
    float result = 0.0f;
    if (idx != -1)
    {
        shadowPos.xyz /= shadowPos.w;
        float depth = shadowPos.z;
    
        uint width, height, mips;
        gShadowMap[idx].GetDimensions(0, width, height, mips);
    
        float dx = 1.0f / (float) width;
        float dy = 1.0f / (float) height;
        const float2 offsets[9] =
        {
            float2(-dx, -dy), float2(0.0f, -dy), float2(+dx, -dy),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(+dx, 0.0f),
        float2(-dx, +dy), float2(0.0f, +dy), float2(+dx, +dy)
        };

        [unroll]
        for (int i = 0; i < 9; i++)
        {
            result += gShadowMap[idx].SampleCmpLevelZero(
            gPCFShadow, shadowPos.xy + offsets[i], depth).r;
        }
        result = result / 9.0f;
    }
    return result;
}