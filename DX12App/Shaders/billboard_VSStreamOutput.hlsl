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

VertexIn VSStreamOutput(VertexIn vin)
{
    return (vin);
}