
Texture2D<float4> inputTexture : register(t0);
RWTexture2D<float4> outputTexture : register(u0);

cbuffer ThresholdParams : register(b0)
{
    float threshold;
}

[numthreads(8, 8, 1)]
void ThresholdAndDownsample(uint3 dispatchID : SV_DispatchThreadID)
{
    uint2 pixel = uint2(dispatchID.x, dispatchID.y);
    
    uint2 inPixel = pixel * 2;
    float4 hIntensity0 = lerp(inputTexture[inPixel], inputTexture[inPixel + uint2(1, 0)], 0.5);
    float4 hIntensity1 = lerp(inputTexture[inPixel + uint2(0, 1)], inputTexture[inPixel + uint2(1, 1)], 0.5);
    float4 intensity = lerp(hIntensity0, hIntensity1, 0.5);
    
    float intensityTest = (float) (length(intensity.rgb) > threshold);

    outputTexture[pixel] = float4(intensityTest * intensity.rgb, 1.0);
}