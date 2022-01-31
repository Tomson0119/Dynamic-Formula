#include "common.hlsl"

Texture2D gDiffuseMap : register(t0);

struct VertexIn
{
	float3 PosL		: POSITION;
    float3 NormalL	: NORMAL;
    float3 TangentL : TANGENT;
    float2 TexCoord : TEXCOORD;
};

struct VertexOut
{
	float4 PosH     : SV_POSITION;
    float3 PosW     : POSITION0;
    float3 oldPosWVP: POSITION1;
    float3 NormalW  : NORMAL;
    float3 TangentW : TANGENT;
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
	
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;
    vout.PosH = mul(posW, gViewProj);

    float4 oldPosWVP = mul(mul(mul(float4(vin.PosL, 1.0f), gOldWorld), gOldView), gProj);
    vout.oldPosWVP = oldPosWVP.xyz;

    float4x4 tWorld = transpose(gWorld);
    vout.NormalW = mul((float3x3)tWorld, vin.NormalL);
    vout.TangentW = mul((float3x3)tWorld, vin.TangentL);
    
    float4 texC = mul(float4(vin.TexCoord, 0.0f, 1.0f), gTexTransform);
    vout.TexCoord = texC.xy;
	
	return vout;
}

PixelOut PS(VertexOut pin)
{
    PixelOut pout;

    int idx = -1;
    float4 PosV = mul(float4(pin.PosW, 1.0f), gView);
    float zSplits[3] = { gZSplit0, gZSplit1, gZSplit2 };
    for (int j = 2; j >= 0; j--)
    {
        if (PosV.z < zSplits[j])
        {
            idx = j;
        }
    }
    float4 PosS = mul(float4(pin.PosW, 1.0f), gShadowTransform[idx]);

    float4 diffuse = gDiffuseMap.Sample(gAnisotropicWrap, pin.TexCoord) * gMat.Diffuse;
    
    float3 view = normalize(gCameraPos - pin.PosW);
    float4 ambient = gAmbient * float4(gMat.Ambient, 1.0f) * diffuse;
    

    float shadowFactor[3] = { 1.0f, 1.0f, 1.0f };
    for (int i = 0; i < 3; i++)
    {
        shadowFactor[i] = CalcShadowFactor(PosS, idx);
    }
    
    float4 directLight;
    float shadowFactorOut[3] = { 1.0f, 1.0f, 1.0f };
    if (PosS.x < 0.0f || PosS.x > 1.0f || PosS.z < 0.0f || PosS.z > 1.0f || PosS.y < 0.0f || PosS.y > 1.0f || idx == -1)
        directLight = ComputeLighting(gLights, gMat, normalize(pin.NormalW), view, shadowFactorOut);
    else
    {
        directLight = ComputeLighting(gLights, gMat, normalize(pin.NormalW), view, shadowFactor);
    }
    float4 result = ambient + directLight;
    result.a = diffuse.a;

    float4 debugColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    //if (idx == 2)
    //{
    //    debugColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
    //}
    //else if (idx == 1)
    //{
    //    debugColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
    //}
    //else if (idx == 0)
    //{
    //    debugColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
    //}
    result *= debugColor;

    pout.f4Color = result;
    pout.f4Direction = distance(din.oldPosWVP, din.PosH.xyz);

    return pout;
}