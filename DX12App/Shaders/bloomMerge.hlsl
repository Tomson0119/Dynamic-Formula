Texture2D<float4> inputTexture0 : register(t0);
Texture2D<float4> inputTexture1 : register(t1);
RWTexture2D<float4> outputTexture : register(u0);

cbuffer CompositeParams : register(b0)
{
    float coefficient;
}

SamplerState texSampler : register(s0);

[numthreads(32, 32, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
    float w, h;
    outputTexture.GetDimensions(w, h);
    float2 uv = float2(dispatchID.x / w, dispatchID.y / h);
    
    uint2 pixel = uint2(dispatchID.x, dispatchID.y);
    
    outputTexture[pixel] = mad(coefficient, inputTexture1.SampleLevel(texSampler, uv, 0), inputTexture0.SampleLevel(texSampler, uv, 0));
}