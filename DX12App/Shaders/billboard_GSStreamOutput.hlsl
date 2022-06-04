#include "common.hlsl"

#define PARTICLE_TYPE_EMMITER 0
#define PARTICLE_TYPE_FLARE 1

Texture2DArray gTexture : register(t0);

struct VertexIn
{
    float3 PosL         : POSITION;
    float2 Size         : SIZE;
    float4 Color        : COLOR;
    float3 Velocity     : VELOCITY;
    float2 Age          : LIFETIME;
    float3 Acceleration : ACCELERATION;
    uint   Type         : TYPE;
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

[maxvertexcount(2)]
void GSStreamOutput(point VertexIn gin[1],
                    inout PointStream<VertexIn> pointStream)
{
    VertexIn particle = gin[0];
    
    particle.Age.x += gElapsedTime;
    
    if (particle.Type == PARTICLE_TYPE_EMMITER)
    {
        if(gParticleEnable && particle.Age.x > particle.Age.y)
        {
            float randFloat[3] = { gRandFloat4.x, gRandFloat4.y, gRandFloat4.z };
            
            particle.Age.x = 0.0f;
            particle.PosL = float3(0, 0, 0);
            pointStream.Append(particle);
            
            for (int i = 0; i < 3; ++i)
            {
                VertexIn vertex = (VertexIn) 0;
                vertex.PosL = particle.PosL;
                vertex.Size = particle.Size;
                vertex.Color = particle.Color;
            
                vertex.Velocity = particle.Velocity;
                
                vertex.Velocity.x += randFloat[i];
                vertex.Velocity.y *= gRandFloat4.w;
                vertex.Velocity.z *= gRandFloat4.w;
                
                vertex.Acceleration = particle.Acceleration;
                
                vertex.Type = PARTICLE_TYPE_FLARE;
                vertex.Age.x = 0.0f;
                vertex.Age.y = 0.5f;
                pointStream.Append(vertex);
                
                for (int j = 0; j < 2; ++j)
                {
                    vertex.PosL = vertex.PosL - vertex.Velocity * 0.01f;
                    
                    pointStream.Append(vertex);
                }
            }
        }
        else
        {
            particle.PosL = float3(0, 0, 0);
            pointStream.Append(particle);
        }
    }
    else if (particle.Type == PARTICLE_TYPE_FLARE && particle.Age.x <= particle.Age.y)
    {
        particle.Velocity.x = particle.Velocity.x + particle.Acceleration.x * gElapsedTime;
        particle.Velocity.y = particle.Velocity.y + particle.Acceleration.y * gElapsedTime;
        particle.Velocity.z = particle.Velocity.z + particle.Acceleration.z * gElapsedTime;
        particle.PosL += particle.Velocity * gElapsedTime;
        pointStream.Append(particle);
    }
}