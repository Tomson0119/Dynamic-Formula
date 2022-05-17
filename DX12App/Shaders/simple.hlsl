#include "common.hlsl"

struct VertexIn
{
    float3 PosL     : POSITION;
};

struct VertexOut
{
    float4 PosH      : SV_POSITION;
    float3 PosW      : POSITION0;
};

struct PixelOut
{
    float4 f4Color : SV_TARGET0;
    float4 f4Direction : SV_TARGET1;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
	
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;
    vout.PosH = mul(posW, gViewProj);
    
    return vout;
}

PixelOut PS(VertexOut pin)
{
    float4 PosV = mul(float4(pin.PosW, 1.0f), gView);
    
    PixelOut pout;
    
    pout.f4Color = float4(0.0f, 1.0f, 0.0f, 1.0f);
    
    pout.f4Direction = float4(0.0f, 0.0f, 0.0f, 0.0f);
    pout.f4Direction.z = PosV.z;
    
    return pout;
}