#include "lighting.hlsl"

Texture2D<float4> inputTexture : register(t0);
Texture2D<float> depthTexture : register(t1);
RWTexture2D<float4> outputTexture : register(u0);

struct VolumetricInfo
{
    float3 Direction;
    int Type;
    float3 Position;
    float Range;
    float3 Color;
    float innerCosine;
    float VolumetricStrength;
    float outerCosine;
};

cbuffer VolumetricCB : register(b1)
{
    matrix gInvProj : packoffset(c0);
    matrix gView : packoffset(c4);
    
    VolumetricInfo gLights[NUM_LIGHTS] : packoffset(c8);
}

float3 GetPositionVS(float2 texcoord, float depth)
{
    float4 clipSpaceLocation;
    clipSpaceLocation.xy = texcoord * 2.0f - 1.0f;
    clipSpaceLocation.y *= -1;
    clipSpaceLocation.z = depth;
    clipSpaceLocation.w = 1.0f;
    float4 homogenousLocation = mul(clipSpaceLocation, gInvProj);
    return homogenousLocation.xyz / homogenousLocation.w;
}

float ExponentialFog(float dist)
{
    float fog_dist = max(dist - 1.0f, 0.0);
    
    float fog = exp(-fog_dist * 10.0f);
    return 1 - fog;
}

[numthreads(32, 32, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
    float4 result = { 0.0f, 0.0f, 0.0f, 0.0f };
    
    uint2 pixel = dispatchID.xy;
    
    uint width, height;
    
    inputTexture.GetDimensions(width, height);
    
    float2 texCoord = (float2(pixel) + 0.5f) / float2(width, height);
    float depth = depthTexture[pixel].r;
    float3 screenPos = GetPositionVS(texCoord, depth);
    float3 rayEnd = float3(0.0f, 0.0f, 0.0f);
	
    const uint sampleCount = 16;
    const float stepSize = length(screenPos - rayEnd) / sampleCount;
    
	// Perform ray marching to integrate light volume along view ray:
	[loop]
    for (uint i = 0; i < NUM_LIGHTS; ++i)
    {
        [branch]
        if (gLights[i].Type == SPOT_LIGHT)
        {
            float3 V = float3(0.0f, 0.0f, 0.0f) - screenPos;
            float cameraDistance = length(V);
        
            V /= cameraDistance;
    
            float marchedDistance = 0;
            float accumulation = 0;
        
            float3 P = screenPos + V * stepSize * dither(pixel.xy);
        
            for (uint j = 0; j < sampleCount; ++j)
            {
                float3 L = mul(float4(gLights[i].Position, 1.0f), gView).xyz - P;
                const float dist2 = dot(L, L);
                const float dist = sqrt(dist2);
                L /= dist;

                float3 viewDir = mul(float4(gLights[i].Direction, 0.0f), gView).xyz;
                //float3 viewDir = gLights[i].Direction;
                
                float SpotFactor = dot(L, -normalize(viewDir));
                float spotCutOff = gLights[i].outerCosine;

		        [branch]
                if (SpotFactor > spotCutOff)
                {
                    float attenuation = DoAttenuation(dist, gLights[i].Range);
                
                    float conAtt = saturate((SpotFactor - gLights[i].outerCosine) / (gLights[i].innerCosine - gLights[i].outerCosine));
                    conAtt *= conAtt;

                    attenuation *= conAtt;
                    
                    attenuation *= ExponentialFog(cameraDistance - marchedDistance);
                    
                    accumulation += attenuation;
                }

                marchedDistance += stepSize;
                P = P + V * stepSize;
            }
            accumulation /= sampleCount;
    
            result += max(0, float4(accumulation * gLights[i].Color * gLights[i].VolumetricStrength, 1));
        }
    }
    
    outputTexture[pixel] = inputTexture[pixel] + result;
}