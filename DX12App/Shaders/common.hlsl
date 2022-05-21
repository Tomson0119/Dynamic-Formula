#include "lighting.hlsl"

Texture2D gShadowMap[3] : register(t0, space1);
TextureCube gCubeMap : register(t0, space2);

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
    matrix gOldViewProj     : packoffset(c13);
}

cbuffer LightCB : register(b1)
{
    matrix gShadowTransform[3] : packoffset(c0); // need as much as shadow map
    float4 gAmbient            : packoffset(c12);
    int    gNumLights          : packoffset(c13);
    Light  gLights[MAX_LIGHTS] : packoffset(c14);
}

cbuffer GameInfoCB : register(b2)
{
    float4 gRandFloat4    : packoffset(c0);
    float  gCurrentTime   : packoffset(c1.x);
    float  gElapsedTime   : packoffset(c1.y);
}

cbuffer MaterialCB : register(b3)
{
    matrix gTexTransform : packoffset(c0);
    Material gMat        : packoffset(c4);    
}

cbuffer ObjectCB : register(b4)
{
    matrix gWorld : packoffset(c0);
    matrix gOldWorld : packoffset(c4);
    bool gCubemapOn : packoffset(c8.x);
    bool gMotionBlurOn : packoffset(c8.y);
    bool gRimLightOn : packoffset(c8.z);
    bool gInvincibleOn : packoffset(c8.w);
}

cbuffer StandardCB : register(b5)
{
    float gZSplit0 : packoffset(c0.x);
    float gZSplit1 : packoffset(c0.y);
    float gZSplit2 : packoffset(c0.z);
    uint gInstancingOffset : packoffset(c0.w);
    bool gParticleEnable : packoffset(c1.x);
}

struct InstancingInfo
{
    matrix World;
    matrix OldWorld;
};
StructuredBuffer<InstancingInfo> gInstancingInfo : register(t0, space3);

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
            result += gShadowMap[idx].SampleCmpLevelZero(gPCFShadow, shadowPos.xy + offsets[i], depth).r;
        }
        result = result / 9.0f;
    }
    return result;
}