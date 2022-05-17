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

PixelOut PS(VertexOut pin, uint primID : SV_PrimitiveID)
{
    PixelOut pout;

    float3 uvw = float3(pin.TexCoord, primID / 2);
    float4 diffuse = gTexture.Sample(gAnisotropicClamp, uvw) * gMat.Diffuse;
    
    pout.f4Color = diffuse;
    pout.f4Direction = float4(0.0f, 0.0f, 0.0f, 0.0f);

    return pout;
}