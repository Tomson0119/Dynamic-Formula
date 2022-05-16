#include "common.hlsli"

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
};

struct VertexOut
{
    float3 PosL : POSITION0;
    float3 PosW : POSITION1;
    float3 NormalL : NORMAL;
};

struct HsConstant
{
    float TessEdges[4] : SV_TessFactor;
    float TessInsides[2] : SV_InsideTessFactor;
};

struct HsOut
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
};

struct DsOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION0;
    float4 PosS : POSITION1;
    float3 NormalW : NORMAL;
    float4 Tessellation : TEXCOORD2;
};

struct DepthOut
{
    float Position : SV_Target;
    float Depth : SV_Depth;
};


VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    vout.PosL = vin.PosL;
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
    vout.NormalL = vin.NormalL;
    return vout;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(25)]
[patchconstantfunc("HSConstant")]
[maxtessfactor(64.0f)]
HsOut HS(InputPatch<VertexOut, 25> hin, uint i : SV_OutputControlPointID)
{
    HsOut hout;
    hout.PosL = hin[i].PosL;
    hout.NormalL = hin[i].NormalL;
    return hout;
}

float CalculateLODTessFactor(float3 pos)
{
    float distToCamera = distance(pos, gCameraPos);
    float s = saturate((distToCamera - 10.0f) / (500.0f - 10.0f));
    return lerp(64.0f, 1.0f, s);
}

HsConstant HSConstant(InputPatch<VertexOut, 25> hin)
{
    HsConstant hconst;
   
    float3 e0 = 0.5f * (hin[0].PosW + hin[4].PosW);
    float3 e1 = 0.5f * (hin[0].PosW + hin[20].PosW);
    float3 e2 = 0.5f * (hin[4].PosW + hin[24].PosW);
    float3 e3 = 0.5f * (hin[20].PosW + hin[24].PosW);
    
    hconst.TessEdges[0] = CalculateLODTessFactor(e0);
    hconst.TessEdges[1] = CalculateLODTessFactor(e1);
    hconst.TessEdges[2] = CalculateLODTessFactor(e2);
    hconst.TessEdges[3] = CalculateLODTessFactor(e3);
    
    float3 sum = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 25; i++)
        sum += hin[i].PosW;
    float3 center = sum / 25.0f;
    hconst.TessInsides[0] = hconst.TessInsides[1] = CalculateLODTessFactor(center);
    
    return hconst;
}

void BernsteinCoeffcient5x5(float t, out float bernstein[5])
{
    float inv_t = 1.0f - t;
    bernstein[0] = inv_t * inv_t * inv_t * inv_t;
    bernstein[1] = 4.0f * t * inv_t * inv_t * inv_t;
    bernstein[2] = 6.0f * t * t * inv_t * inv_t;
    bernstein[3] = 4.0f * t * t * t * inv_t;
    bernstein[4] = t * t * t * t;
}

float3 CubicBezierSum5x5(OutputPatch<HsOut, 25> patch, float uB[5], float vB[5], bool normal)
{
    float3 sum = float3(0.0f, 0.0f, 0.0f);
    
    int i = 0;
    [unroll]
    for (i = 0; i < 5; i++)
    {
        float3 bu = float3(0.0f, 0.0f, 0.0f);
        int j = 0;
        [unroll]
        for (j = 0; j < 5; j++)
        {
            if (normal == false) 
                bu += uB[j] * patch[5 * i + j].PosL;
            else
                bu += uB[j] * patch[5 * i + j].NormalL;
        }
        sum += vB[i] * bu;
    }
    return sum;
}

[domain("quad")]
DsOut DS(HsConstant hconst, float2 uv : SV_DomainLocation, OutputPatch<HsOut, 25> patch)
{
    DsOut dout = (DsOut) 0;
    
    float uB[5], vB[5];
    BernsteinCoeffcient5x5(uv.x, uB);
    BernsteinCoeffcient5x5(uv.y, vB);
    
    float3 pos = CubicBezierSum5x5(patch, uB, vB, false);
    float4 posW = mul(float4(pos, 1.0f), gWorld);
    
    dout.PosH = mul(mul(float4(pos, 1.0f), gWorld), gViewProj);
    float3 normalL = CubicBezierSum5x5(patch, uB, vB, true);
    float4x4 tWorld = transpose(gWorld);
    dout.NormalW = mul((float3x3) tWorld, normalize(normalL));
    dout.Tessellation = float4(hconst.TessEdges[0], hconst.TessEdges[1], hconst.TessEdges[2], hconst.TessEdges[3]);
    
    return dout;
}