#include "common.hlsl"

struct VertexIn
{
	float3 PosL  	: POSITION;
};

struct VertexOut
{
	float4 PosH     : SV_POSITION;
};

VertexOut VS(VertexIn vin, uint InstanceID : SV_InstanceID)
{
    VertexOut vout;
	
    float4 posW = mul(float4(vin.PosL, 1.0f), gInstancingInfo[InstanceID + gInstancingOffset].World);
    vout.PosH = mul(posW, gViewProj);
	
    return vout;
}

float4 PS(VertexOut pin) : SV_Position
{
    float4 pout = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    return pout;
}