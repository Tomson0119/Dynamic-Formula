///////////////////////////////////////////////////////////////////////////////
//
#define FRAME_WIDTH  1920
#define FRAME_HEIGHT 1080

Texture2D VelocityMap : register(t0);
Texture2D RenderTarget : register(t1);

RWTexture2D<float4> RWOutput : register(u0);

cbuffer CommonCB : register(b0)
{
    int gFrameWidth : packoffset(c0.x);
    int gFrameHeight : packoffset(c0.y);
}

[numthreads(32, 32, 1)]
void CS(int3 n3GroupThreadID : SV_GroupThreadID, int3 n3DispatchThreadID : SV_DispatchThreadID)
{
    int numSample = 50;

    float Depth = VelocityMap[int2(n3DispatchThreadID.x, n3DispatchThreadID.y)].b;

    int BlurConst = 1;

    float Blur = VelocityMap[int2(n3DispatchThreadID.x, n3DispatchThreadID.y)].a;
    float2 Velocity = -VelocityMap[int2(n3DispatchThreadID.x, n3DispatchThreadID.y)].rg * BlurConst;

    float4 cColor = RenderTarget[int2(n3DispatchThreadID.x, n3DispatchThreadID.y)];

    int cnt = 0;
    if (Blur && (abs(length(Velocity)) > 0.01f))
    {
        for (int i = 0; i < numSample; ++i)
        {
            if (n3DispatchThreadID.x + round(Velocity.x * i) >= 0 && n3DispatchThreadID.x + round(Velocity.x * i) < gFrameWidth - 1 &&
                n3DispatchThreadID.y + round(Velocity.y * i) >= 0 && n3DispatchThreadID.y + round(Velocity.y * i) < gFrameHeight - 1)
            {
                cColor += RenderTarget[int2(n3DispatchThreadID.x + 1 + round(Velocity.x * i), n3DispatchThreadID.y + 1 + round(Velocity.y * i))];
                cnt++;
            }
        }
    }
    cColor /= cnt + 1;

    RWOutput[n3DispatchThreadID.xy] = cColor;
}