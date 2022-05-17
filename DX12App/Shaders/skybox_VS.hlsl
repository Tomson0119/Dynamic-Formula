#include "common.hlsl"

Texture2DArray gTexture : register(t0);

struct VertexIn
{
	float3 PosL		: POSITION;
    float3 NormalL	: NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct VertexOut
{
	float4 PosH     : SV_POSITION;
    float3 PosW   : POSITION;
    float3 NormalW  : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct PixelOut
{
    float4 f4Color : SV_TARGET0;
    float4 f4Direction : SV_TARGET1;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
    vout.PosW.xyz += gCameraPos;
  
    vout.PosH = mul(float4(vout.PosW, 1.0f), gViewProj);
    
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorld);
    vout.TexCoord = vin.TexCoord;
	
	return vout;
}
