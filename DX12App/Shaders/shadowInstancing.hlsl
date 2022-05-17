#include "common.hlsl"

struct VertexIn
{
    float3 PosL : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

struct DepthOut
{
    float Position : SV_Target;
    float Depth : SV_Depth;
};

VertexOut VS(VertexIn vin, uint InstanceID : SV_InstanceID)
{
    VertexOut vout;
	
    float4 posW = mul(float4(vin.PosL, 1.0f), gInstancingInfo[InstanceID + gInstancingOffset].World);
    vout.PosH = mul(posW, gViewProj);

    float4 texC = mul(float4(vin.TexCoord, 0.0f, 1.0f), gTexTransform);
    vout.TexCoord = texC.xy;
	
    return vout;
}

DepthOut PS(VertexOut pin)
{
    DepthOut dout = (DepthOut) 0;
    dout.Position = pin.PosH.z;
    dout.Depth = pin.PosH.z;
    return dout;
}