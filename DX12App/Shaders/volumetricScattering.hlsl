#include "lighting.hlsl"

Texture2D<float4> inputTexture : register(t0);
Texture2D<float> depthTexture : register(t1);
RWTexture2D<float4> outputTexture : register(u0);

cbuffer VolumetricCB : register(b1)
{
    matrix gInvProj : packoffset(c0);
    matrix gInvView : packoffset(c4);
    
    float gVolumetricStrength : packoffset(c8.x);
    
    Light gLights[NUM_LIGHTS] : packoffset(c9);
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

[numthreads(32, 32, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
    float4 result = { 0.0f, 0.0f, 0.0f, 0.0f };
    
    uint2 pixel = dispatchID.xy;
    
    uint width, height;
    
    inputTexture.GetDimensions(width, height);
    
    float2 texCoord = pixel / float2(width, height);
    float depth = depthTexture[pixel].r;
    float3 screenPos = GetPositionVS(texCoord, depth);
    float3 rayEnd = float3(0.0f, 0.0f, 0.0f);
	
    const uint sampleCount = 16;
    const float stepSize = length(screenPos - rayEnd) / sampleCount;
    
	// Perform ray marching to integrate light volume along view ray:
    
    uint spotNum = 0;
	[loop]
    for (uint i = 0; i < NUM_LIGHTS; ++i)
    {
        [branch]
        if (gLights[i].Type == SPOT_LIGHT)
        {
            spotNum++;
            float3 V = float3(0.0f, 0.0f, 0.0f) - screenPos;
            float cameraDistance = length(V);
        
            V /= cameraDistance;
    
            float marchedDistance = 0;
            float accumulation = 0;
        
            float3 P = screenPos + V * stepSize * dither(pixel.xy);
        
            for (uint j = 0; j < sampleCount; ++j)
            {
                float3 L = gLights[i].Position.xyz - P;
                const float dist2 = dot(L, L);
                const float dist = sqrt(dist2);
                L /= dist;

                float SpotFactor = dot(L, normalize(-gLights[i].Direction.xyz));

		        [branch]
                if (dist > gLights[i].FalloffEnd)
                {
                    float attenuation = CalcAttenuation(dist, gLights[i].FalloffStart, gLights[i].FalloffEnd);
                
                    //float conAtt = saturate((SpotFactor - light_cbuf.current_light.outer_cosine) / (light_cbuf.current_light.inner_cosine - light_cbuf.current_light.outer_cosine));
                    //conAtt *= conAtt;

                    //attenuation *= conAtt;

                    accumulation += attenuation;
                }

                marchedDistance += stepSize;
                P = P + V * stepSize;
            }
            accumulation /= sampleCount;
    
            result += max(0, float4(accumulation * gLights[i].Diffuse.rgb * gVolumetricStrength, 1));
        }
    }
    
    outputTexture[pixel] = result / spotNum;
}