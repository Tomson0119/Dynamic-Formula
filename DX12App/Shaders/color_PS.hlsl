#include "common.hlsl"

struct VertexIn
{
    float3 PosL     : POSITION;
    float3 NormalL  : NORMAL;
    float3 TangentL : TANGENT;
    float2 TexCoord : TEXCOORD;
};

struct VertexOut
{
    float4 PosH      : SV_POSITION;
    float3 PosW      : POSITION0;
    float4 oldPosWVP : POSITION1;
    float4 newPosWVP : POSITION2;
    float3 NormalW   : NORMAL;
    float3 TangentW  : TANGENT;
    float2 TexCoord  : TEXCOORD;
};

struct PixelOut
{
    float4 f4Color : SV_TARGET0;
    float4 f4Direction : SV_TARGET1;
};

PixelOut PS(VertexOut pin)
{
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

    float3 view = normalize(gCameraPos - pin.PosW);
    float4 ambient = gAmbient * float4(gMat.Ambient, 1.0f) * gMat.Diffuse;
    
    float shadowFactor = CalcShadowFactor(PosS, idx);
    float4 directLight;
    float shadowFactorOut = 1.0f;

    if (PosS.x < 0.0f || PosS.x > 1.0f || PosS.z < 0.0f || PosS.z > 1.0f || PosS.y < 0.0f || PosS.y > 1.0f || idx == -1)
        directLight = ComputeLighting(gLights, gMat, pin.PosW, normalize(pin.NormalW), view, shadowFactorOut, gRimLightOn, gNumLights);
    else
        directLight = ComputeLighting(gLights, gMat, pin.PosW, normalize(pin.NormalW), view, shadowFactor, gRimLightOn, gNumLights);
    

    PixelOut pout;

    float4 result = ambient + directLight;
    
    if (gCubemapOn)
    {
        float3 Normal = normalize(pin.NormalW);
        float3 fromEye = normalize(pin.PosW - gCameraPos.xyz);
        float3 reflected = normalize(reflect(fromEye, pin.NormalW));
    
        result = saturate((gCubeMap.Sample(gLinearWrap, reflected) * 0.3f) + (result * 0.7f));
    }
    
    result.a = 1.0f;
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
    
    if(gInvincibleOn)
    {
        pout.f4Color.a = 0.3f;
    }
    
    return pout;
}