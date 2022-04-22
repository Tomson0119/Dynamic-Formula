#include "lighting.hlsl"

Texture2D<float4> inputTexture : register(t0);
Texture2D<float> depthTexture : register(t1);
RWTexture2D<float4> outputTexture : register(u0);

#define PI 3.141592653589793238f

cbuffer VolumetricCB : register(b1)
{
    float absorptionTau : packoffset(c0);
    float3 absorptionColor : packoffset(c0.y);
    int scatteringSamples : packoffset(c1.x);
    float scatteringTau : packoffset(c1.y);
    float scatteringZFar : packoffset(c1.z);
    
    float3 scatteringColor : packoffset(c2);
    
    matrix gInvProj : packoffset(c3);
    matrix gInvView : packoffset(c7);
    float3 gCameraPos : packoffset(c11);
    
    Light gLights[NUM_LIGHTS] : packoffset(c12);
}

float random(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453123);
}


float3 PixelWorldPos(float depthValue, int2 pixel)
{
    uint width, height;
    inputTexture.GetDimensions(width, height);
    
    float2 fPixel = float2(pixel.x, pixel.y);
    
    float x = (fPixel.x / width * 2) - 1;
    float y = (fPixel.y / height * (-2)) + 1;
    float z = depthValue;
    
    float4 ndcCoords = float4(x, y, z, 1.0f);

    float4 p = mul(ndcCoords, gInvProj);
    
    p /= p.w;
    
    float4 worldCoords = mul(p, gInvView);
    
    return worldCoords.xyz;
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

float3 volumetricScattering(float3 worldPosition, Light light)
{
    float3 result = float3(0.0, 0.0, 0.0);
    float3 camToFrag = worldPosition - gCameraPos;
    if (length(camToFrag) > scatteringZFar)
    {
        camToFrag = normalize(camToFrag) * scatteringZFar;
    }
    float3 deltaStep = camToFrag / (scatteringSamples + 1);
    float3 fragToCamNorm = normalize(gCameraPos - worldPosition);
    float3 x = gCameraPos;
    
    float rand = random(worldPosition.xy + worldPosition.z);
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
    float depthValue = depthTexture[pixel].r;
    float3 worldPosition = PixelWorldPos(depthValue, pixel);
    
    float fragCamDist = distance(worldPosition, gCameraPos);
    
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        if(gLights[i].Type == SPOT_LIGHT)
            volumetricColor += float4(volumetricScattering(worldPosition, gLights[i]), 0.0);
    }
    
    outputTexture[pixel] = volumetricColor;
}   

//[numthreads(32, 32, 1)]
//void CS(uint3 dispatchID : SV_DispatchThreadID) : COLOR0
//{ 
//    uint2 pixel = dispatchID.xy;
    
//    for (int i = 0; i < NUM_LIGHTS; ++i)
//    {
//        // Calculate vector from pixel to light source in screen space.
//        half2 deltaTexCoord = (float2(pixel.x / width, pixel.y / height) - mul(mul(gLights[i].Position, gView), gProj).xy);
//        // Divide by number of samples and scale by control factor.
//        deltaTexCoord *= 1.0f / NUM_SAMPLES * Density;
//        // Store initial sample.
//        half3 color = inputTexture[pixel];
//        // Set up illumination decay factor.
//        half illuminationDecay = 1.0f;
//        // Evaluate summation from Equation 3 NUM_SAMPLES iterations.
//        for (int i = 0; i < NUM_SAMPLES; i++)
//        {
//            // Step sample location along ray.
//            texCoord -= deltaTexCoord;
//            // Retrieve sample at new location.
//            half3 sample = tex2D(frameSampler, texCoord);
//            // Apply sample attenuation scale/decay factors.
//            sample *= illuminationDecay * Weight;
//            // Accumulate combined color.
//            color += sample;
//            // Update exponential decay factor.
//            illuminationDecay *= Decay;
//        }
//        // Output final color with a further scale control factor.
//        outputTexture[pixel] = float4(color * Exposure, 1);
//    }
//} 