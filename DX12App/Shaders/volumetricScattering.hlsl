#include "common.hlsl"

Texture2D<float4> inputTexture : register(t0);
Texture2D<float> depthTexture : register(t1);
RWTexture2D<float4> outputTexture : register(u0);

float3 fragmentWorldPos(float depthValue, int2 dispatchID)
{
    uint width, height;
    inputTexture.GetDimensions(width, height);
    
    float4 ndcCoords = float4(
        2.0 * (dispatchID.x / width) - 1.0,
        2.0 * (dispatchID.y / height) - 1.0,
        2.0 * depthValue - 1.0,
        1.0);

    float4 worldCoords = mul(ndcCoords, gInvViewProj);
    
    return worldCoords.xyz / worldCoords.w;
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