#include "common.hlsl"

#define PARTICLE_TYPE_EMMITER 0
#define PARTICLE_TYPE_FLARE 1

Texture2DArray gTexture : register(t0);

struct VertexIn
{
    float3 PosL      : POSITION;
    float2 Size      : SIZE;
    float4 Color     : COLOR;
    float3 Velocity  : VELOCITY;
    float2 Age       : LIFETIME;
    uint   Type      : TYPE;
};

struct GeoOut
{
    float4 PosH     : SV_POSITION;
    uint   PrimID   : SV_PrimitiveID;
    float4 Color    : COLOR;
    float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
    float2 TexCoord : TEXCOORD;
    float2 Age      : LIFETIME;
    uint   Type     : TYPE;
};

[maxvertexcount(4)]
void GSRender(point VertexIn gin[1],
        uint primID : SV_PrimitiveID,
        inout TriangleStream<GeoOut> triStream)
{
    if (gin[0].Type == PARTICLE_TYPE_FLARE)
    {
        float3 posW = mul(float4(gin[0].PosL, 1.0f), gWorld).xyz;
    
        float3 up = float3(0.0f, 1.0f, 0.0f);
        float3 look = gCameraPos - posW;
    
        look.y = 0.0f;
        look = normalize(look);
    
        float3 right = cross(up, look);
        
        float sizeScale = (1 - (gin[0].Age.x / gin[0].Age.y));
        float hw = gin[0].Size.x * 0.5f * sizeScale;
        float hh = gin[0].Size.y * 0.5f * sizeScale;
    
        float4 v[4];
        v[0] = float4(posW + hw * right - hh * up, 1.0f);
        v[1] = float4(posW + hw * right + hh * up, 1.0f);
        v[2] = float4(posW - hw * right - hh * up, 1.0f);
        v[3] = float4(posW - hw * right + hh * up, 1.0f);
    
        float2 TexCoord[4] =
        {
            float2(0.0f, 1.0f),
            float2(0.0f, 0.0f),
            float2(1.0f, 1.0f),
            float2(1.0f, 0.0f)
        };

        GeoOut gout;
        [unroll]
        for (int i = 0; i < 4; i++)
        {
            gout.PosH = mul(v[i], gViewProj);
            gout.PosW = v[i].xyz;
            gout.NormalW = look;
            gout.TexCoord = TexCoord[i];
            gout.PrimID = primID;
            gout.Type = gin[0].Type;
            gout.Age = gin[0].Age;
            gout.Color = gin[0].Color;
        
            triStream.Append(gout);
        }
    }
}