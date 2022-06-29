#include "common.hlsl"

Texture2D gDiffuseMap : register(t0);

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float3 TangentL : TANGENT;
    float2 TexCoord : TEXCOORD;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION0;
    float4 oldPosWVP : POSITION1;
    float4 newPosWVP : POSITION2;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 TexCoord : TEXCOORD;
};

struct PixelOut
{
    float4 f4Color : SV_TARGET0;
    float4 f4Direction : SV_TARGET1;
};

VertexOut VS(VertexIn vin, uint InstanceID : SV_InstanceID)
{
    VertexOut vout;
	
    float4 posW = mul(float4(vin.PosL, 1.0f), gInstancingInfo[InstanceID + gInstancingOffset].World);
    vout.PosW = posW.xyz;
    vout.PosH = mul(posW, gViewProj);

    vout.oldPosWVP = mul(mul(float4(vin.PosL, 1.0f), gInstancingInfo[InstanceID + gInstancingOffset].OldWorld), gOldViewProj);
    vout.newPosWVP = vout.PosH;

    float4x4 tWorld = transpose(gInstancingInfo[InstanceID + gInstancingOffset].World);
    vout.NormalW = mul(float4(vin.NormalL, 0.0f), gInstancingInfo[InstanceID + gInstancingOffset].World).xyz;
    vout.TangentW = mul((float3x3) tWorld, vin.TangentL);
    
    float4 texC = mul(float4(vin.TexCoord, 0.0f, 1.0f), gTexTransform);
    vout.TexCoord = texC.xy;
	
    return vout;
}
