#include "common.hlsl"

Texture2D<float4> inputTexture : register(t0);
Texture2D<float> depthTexture : register(t1);
RWTexture2D<float4> outputTexture : register(u0);

cbuffer VolumetricCB : register(b0)
{
    float absorptionTau : packoffset(c0);
    float3 absorptionColor : packoffset(c0.y);
    int scatteringSamples : packoffset(c1.x);
    float scatteringTau : packoffset(c1.y);
    float anisotropy : packoffset(c1.z);
    float scatteringZFar : packoffset(c1.w);
    float3 scatteringColor : packoffset(c2);
}

float random(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453123);
}


float3 fragmentWorldPos(float depthValue, int2 dispatchID)
{
    uint width, height;
    inputTexture.GetDimensions(width, height);
    
    float4 ndcCoords = float4(
        dispatchID.x / width,
        dispatchID.y / height,
        depthValue,
        1.0);

    float4 worldCoords = mul(ndcCoords, gInvViewProj);
    
    return worldCoords.xyz / worldCoords.w;
}

float3 absorptionTransmittance(float dist)
{
    return absorptionColor * exp(-dist * (absorptionTau + scatteringTau));
}

float phaseFunction(float3 inDir, float3 outDir)
{
    float cosAngle = dot(inDir, outDir) / (length(inDir) * length(outDir));
    float x = (1.0 + cosAngle) / 2.0;
    float x2 = x * x;
    float x4 = x2 * x2;
    float x8 = x4 * x4;
    float x16 = x8 * x8;
    float x32 = x16 * x16;
    float nom = 0.5 + 16.5 * x32;
    float factor = 1.0 / (4.0 * PI);
    return nom * factor;
}

float3 volumetricScattering(float3 fragPosition, Light light)
{
    float3 result = float3(0.0, 0.0, 0.0);
    float3 camToFrag = fragPosition - gCameraPos;
    if (length(camToFrag) > scatteringZFar)
    {
        camToFrag = normalize(camToFrag) * scatteringZFar;
    }
    float3 deltaStep = camToFrag / (scatteringSamples + 1);
    float3 fragToCamNorm = normalize(gCameraPos - fragPosition);
    float3 x = gCameraPos;
    
    float rand = random(fragPosition.xy + fragPosition.z);
    x += (deltaStep * rand);
    
    for (int i = 0; i < scatteringSamples; ++i)
    {
        float visibility = 1.0;
        float3 lightToX = x - light.Position;
        float lightDist = length(lightToX);
        float omega = 4 * PI * lightDist * lightDist;
        float3 Lin = absorptionTransmittance(lightDist) * visibility * light.Diffuse * light.SpotPower / omega;
        float3 Li = Lin * scatteringTau * scatteringColor * phaseFunction(normalize(lightToX), fragToCamNorm);
        result += Li * absorptionTransmittance(distance(x, gCameraPos)) * length(deltaStep);
        x += deltaStep;
    }
    
    return result;
}

[numthreads(32, 32, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
    int2 pixel = int2(dispatchID.x, dispatchID.y);
    
    float4 volumetricColor = float4(0.0, 0.0, 0.0, 1.0);
    float depthValue = depthTexture[pixel];
    float3 fragPosition = fragmentWorldPos(depthValue, pixel);
    
    float fragCamDist = distance(fragPosition, gCameraPos);
    
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        volumetricColor += float4(volumetricScattering(fragPosition, gLights[i]), 0.0);
    }
    
    
    outputTexture[pixel] = volumetricColor;
}