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
    float4 oldPosWVP : POSITION1;
    float4 newPosWVP : POSITION2;
    float3 NormalW  : NORMAL;
    float3 TangentW : TANGENT;
    float2 TexCoord : TEXCOORD;
};

struct PixelOut
{
    float4 f4Color : SV_TARGET0;
    float4 f4Direction : SV_TARGET1;
};

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
    

    float shadowFactor = CalcShadowFactor(PosS, idx);
    
    float4 directLight;
    float shadowFactorOut = 1.0f;
    if (PosS.x < 0.0f || PosS.x > 1.0f || PosS.z < 0.0f || PosS.z > 1.0f || PosS.y < 0.0f || PosS.y > 1.0f || idx == -1)
        directLight = ComputeLighting(gLights, gMat, pin.PosW, normalize(pin.NormalW), view, shadowFactorOut, gRimLightOn);
    else
    {
        directLight = ComputeLighting(gLights, gMat, pin.PosW, normalize(pin.NormalW), view, shadowFactor, gRimLightOn);
    }
    float4 result = ambient + directLight;
    result.a = diffuse.a;

    pout.f4Color = result;
    if (gMotionBlurOn)
    {
        pout.f4Direction = float4((pin.newPosWVP.xyz / pin.newPosWVP.z) - (pin.oldPosWVP.xyz / pin.oldPosWVP.z), 1.0f);
        pout.f4Direction.z = PosV.z;
    }
    else
    {
        pout.f4Direction = float4(0.0f, 0.0f, 0.0f, 0.0f);
        pout.f4Direction.z = PosV.z;
    }

    return pout;
}