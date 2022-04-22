#include "lighting.hlsl"

Texture2D<float4> inputTexture : register(t0);
Texture2D<float> depthTexture : register(t1);
RWTexture2D<float4> outputTexture : register(u0);

#define PI 3.141592653589793238f

//cbuffer VolumetricCB : register(b1)
//{
//    float absorptionTau : packoffset(c0);
//    float3 absorptionColor : packoffset(c0.y);
//    int scatteringSamples : packoffset(c1.x);
//    float scatteringTau : packoffset(c1.y);
//    float scatteringZFar : packoffset(c1.z);
    
//    float3 scatteringColor : packoffset(c2);
    
//    matrix gInvProj : packoffset(c3);
//    matrix gInvView : packoffset(c7);
//    float3 gCameraPos : packoffset(c11);
    
//    Light gLights[NUM_LIGHTS] : packoffset(c12);
//}

//float random(float2 co)
//{
//    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453123);
//}

//float shadowFactor(Light light, float3 p)
//{
//    float result = 1.0f;
//    if (light.Type == SPOT_LIGHT)
//    {
//        float3 fragLightDir = normalize(light.Position - p);
//        float cosAngle = dot(fragLightDir, normalize(-light.Direction));
//        if(cosAngle > light.spotCutOff)
//        {
//            float4 pointLightSpace = light.spotWorldToProj * float4(p, 1.0);
//            float3 lightProjCoords = pointLightSpace.xyz / pointLightSpace.w;
//            lightProjCoords = lightProjCoords * 0.5 + 0.5;
//            float shadowDist = light.spotShadowMap[lightProjCoords.xy];
//            float lightDist = lightProjCoords.z;
//            result = lightDist > shadowDist + SPOT_SHADOW_BIAS ? 1.0 : 0.0;
//        }
//    }
//    return result;
//}




//float3 PixelWorldPos(float depthValue, int2 pixel)
//{
//    uint width, height;
//    inputTexture.GetDimensions(width, height);
    
//    float2 fPixel = float2(pixel.x, pixel.y);
    
//    float x = (fPixel.x / width * 2) - 1;
//    float y = (fPixel.y / height * (-2)) + 1;
//    float z = depthValue;
    
//    float4 ndcCoords = float4(x, y, z, 1.0f);

//    float4 p = mul(ndcCoords, gInvProj);
    
//    p /= p.w;
    
//    float4 worldCoords = mul(p, gInvView);
    
//    return worldCoords.xyz;
//}

//float3 absorptionTransmittance(float dist)
//{
//    return absorptionColor * exp(-dist * (absorptionTau + scatteringTau));
//}

//float phaseFunctionRayleigh(float3 inDir, float3 outDir)
//{
//    float cosAngle = dot(inDir, outDir) / (length(inDir) * length(outDir));
//    float nom = 3.0 * (1.0 + cosAngle * cosAngle);
//    float denom = 16.0 * PI;
//    return nom / denom;
//}

//float phaseFunction(float3 inDir, float3 outDir)
//{
//    float cosAngle = dot(inDir, outDir) / (length(inDir) * length(outDir));
//    float x = (1.0 + cosAngle) / 2.0;
//    float x2 = x * x;
//    float x4 = x2 * x2;
//    float x8 = x4 * x4;
//    float x16 = x8 * x8;
//    float x32 = x16 * x16;
//    float nom = 0.5 + 16.5 * x32;
//    float factor = 1.0 / (4.0 * PI);
//    return nom * factor;
//}

//float3 volumetricScattering(float3 worldPosition, Light light)
//{
//    float3 result = float3(0.0, 0.0, 0.0);
//    float3 camToFrag = worldPosition - gCameraPos;
    
//    float3 lightToFrag = worldPosition - light.Position;
  
//    if (length(camToFrag) > scatteringZFar)
//    {
//        camToFrag = normalize(camToFrag) * scatteringZFar;
//    }
//    float3 deltaStep = camToFrag / (scatteringSamples + 1);
//    float3 fragToCamNorm = normalize(gCameraPos - worldPosition);
//    float3 x = gCameraPos;
    
//    float rand = random(worldPosition.xy + worldPosition.z);
//    x += (deltaStep * rand);
    
//    for (int i = 0; i < scatteringSamples; ++i)
//    {
//        float visibility = 1.0f;
//        float3 lightToX = x - light.Position;
//        float lightDist = length(lightToX) / 2;
//        float omega = 4 * PI * lightDist * lightDist;
//        float3 Lin = absorptionTransmittance(lightDist) * visibility * light.Diffuse * 500 / omega;
//        float3 Li = Lin * scatteringTau * scatteringColor * phaseFunctionRayleigh(normalize(lightToX), fragToCamNorm);
//        result += Li * absorptionTransmittance(distance(x, gCameraPos)) * length(deltaStep);
//        x += deltaStep;
//    }
    
//    return result;
//}

//[numthreads(32, 32, 1)]
//void CS(uint3 dispatchID : SV_DispatchThreadID)
//{
//    int2 pixel = int2(dispatchID.x, dispatchID.y);
    
//    float4 volumetricColor = float4(0.0, 0.0, 0.0, 1.0);
//    float depthValue = depthTexture[pixel].r;
//    float3 worldPosition = PixelWorldPos(depthValue, pixel);
    
//    float fragCamDist = distance(worldPosition, gCameraPos);
    
//    for (int i = 0; i < NUM_LIGHTS; ++i)
//    {
//        if (gLights[i].Type == SPOT_LIGHT && gLights[i].FalloffEnd > length(gLights[i].Position - worldPosition))
//            volumetricColor += float4(volumetricScattering(worldPosition, gLights[i]), 0.0);
//    }
    
//    outputTexture[pixel] = volumetricColor + inputTexture[pixel];
//}   

cbuffer VolumetricCB : register(b1)
{
    int NUM_SAMPLES : packoffset(c0.x);
    float Density : packoffset(c0.y);
    float Exposure : packoffset(c0.z);
    float Weight : packoffset(c0.w);
    matrix gProj : packoffset(c1);
    matrix gView : packoffset(c5);
    
    float Decay : packoffset(c9.x);
    
    float3 gCameraPos : packoffset(c9.y);
    
    Light gLights[NUM_LIGHTS] : packoffset(c10);
}

[numthreads(32, 32, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
    uint2 pixel = dispatchID.xy;
    
    uint width, height;
    inputTexture.GetDimensions(width, height);
    
    float4 result = inputTexture[pixel];
    
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        if(gLights[i].Type == SPOT_LIGHT)
        {
            float2 texCoord = float2(pixel.x / width, pixel.y / height);
        
            float2 deltaTexCoord = (texCoord - mul(mul(float4(gLights[i].Position, 1.0f), gView), gProj).xy);
        
            deltaTexCoord *= 1.0f / NUM_SAMPLES * Density;
        
            float3 color = inputTexture[pixel].rgb;
        
            float illuminationDecay = 1.0f;
        
            for (int j = 0; j < NUM_SAMPLES; j++)
            {
                texCoord -= deltaTexCoord;
                
                uint2 modifiedPixel = uint2(texCoord.x * width, texCoord.y * height);
                float3 sample = inputTexture[modifiedPixel].rgb;
                sample *= illuminationDecay * Weight;
                color += sample;
                illuminationDecay *= Decay;
            }
            result += float4(color * Exposure, 1);
        }
    }
    
    outputTexture[pixel] = result;
}