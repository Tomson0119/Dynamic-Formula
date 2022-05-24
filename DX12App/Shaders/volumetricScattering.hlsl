#include "lighting.hlsl"

Texture2D<float4> inputTexture : register(t0);
Texture2D<float> depthTexture : register(t1);
Texture2D<float> shadowDepthMap[3] : register(t2);
RWTexture2D<float4> outputTexture : register(u0);

SamplerComparisonState gPCFShadow : register(s1);

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
    matrix gShadowTransform[3] : packoffset(c8);
    int gNumLights : packoffset(c20.x);
    float gZSplit1 : packoffset(c20.y);
    float gZSplit2 : packoffset(c20.z);
    float gZSplit3 : packoffset(c20.w);
    VolumetricInfo gLights[MAX_LIGHTS] : packoffset(c21);
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
	
    const uint sampleCount = 12;
	// Perform ray marching to integrate light volume along view ray:
	[loop]
    for (int i = 0; i < gNumLights; ++i)
    {
        [branch]
        if (gLights[i].Type == SPOT_LIGHT || gLights[i].Type == POINT_LIGHT)
        {
            const float stepSize = length(screenPos - rayEnd) / sampleCount;
    
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

                float3 viewDir = mul(float4(normalize(gLights[i].Direction), 0.0f), gView).xyz;
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
        
    //    else if(gLights[i].Type == DIRECTIONAL_LIGHT)
    //    {
    //        float3 V = float3(0.0f, 0.0f, 0.0f) - screenPos;
    //        float cameraDistance = length(V);
    //        V /= cameraDistance;

    //        float marchedDistance = 0;
    //        float3 accumulation = 0;
            
    //        float3 P = screenPos;
    //        const float3 L = normalize(gLights[i].Direction);
            
    //        const float stepSize = length(P - rayEnd) / sampleCount;

	   //     // dither ray start to help with undersampling:
    //        P = screenPos + V * stepSize * dither(pixel.xy);
    //        float viewDepth = P.z;
            
	   //     // Perform ray marching to integrate light volume along view ray:
	   //     [loop]
    //        for (uint i = 0; i < sampleCount; ++i)
    //        {
    //            bool valid = false;
    //            for (uint cascade = 0; cascade < 3; ++cascade)
    //            {
    //                matrix light_space_matrix = gShadowTransform[cascade];
    //                float zSplit = cascade == 0 ? gZSplit1 : cascade == 1 ? gZSplit2 : gZSplit3;
                    
    //                float4 posShadowMap = mul(float4(P, 1.0), light_space_matrix);
        
    //                float3 UVD = posShadowMap.xyz / posShadowMap.w;

    //                UVD.xy = 0.5 * UVD.xy + 0.5;
    //                UVD.y = 1.0 - UVD.y;

    //                [branch]
    //                if (viewDepth < zSplit)
    //                {
    //                    if (IsSaturated(UVD.xy))
    //                    {
    //                        float attenuation = CalcShadowFactor_PCF3x3(gPCFShadow, shadowDepthMap[i], UVD);

    //                        attenuation *= ExponentialFog(cameraDistance - marchedDistance);

    //                        accumulation += attenuation;
    //                    }

    //                    marchedDistance += stepSize;
    //                    P = P + V * stepSize;

    //                    valid = true;
    //                    break;
    //                }
    //            }

    //            if (!valid)
    //            {
    //                break;
    //            }
    //        }

    //        accumulation /= sampleCount;
            
    //        result += max(0, float4(accumulation * gLights[i].Color * gLights[i].VolumetricStrength, 1));
    //    }
    }
    
    outputTexture[pixel] = inputTexture[pixel] + result;
}