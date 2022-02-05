///////////////////////////////////////////////////////////////////////////////
//
#define FRAME_WIDTH  1920
#define FRAME_HEIGHT 1080

Texture2D VelocityMap : register(t0);
Texture2D RenderTarget : register(t1);

RWTexture2D<float4> RWOutput : register(u0);

[numthreads(32, 30, 1)]
void CS(int3 n3GroupThreadID : SV_GroupThreadID, int3 n3DispatchThreadID : SV_DispatchThreadID)
{
    int numSample = 20;

    float Depth = VelocityMap[int2(n3DispatchThreadID.x, n3DispatchThreadID.y)].z;

    int BlurConst = 10000 / Depth;

    float Blur = VelocityMap[int2(n3DispatchThreadID.x, n3DispatchThreadID.y)].w;
    float2 Velocity = VelocityMap[int2(n3DispatchThreadID.x, n3DispatchThreadID.y)].xy * (BlurConst);

    float4 cColor = RenderTarget[int2(n3DispatchThreadID.x + 1, n3DispatchThreadID.y + 1)];

    int cnt = 0;
    if (Blur)
    {
        for (int i = 0; i < numSample; ++i)
        {
            if (n3DispatchThreadID.x + 1 + round(Velocity.x * i) >= 0 && n3DispatchThreadID.x + 1 + round(Velocity.x * i) <= FRAME_WIDTH &&
                n3DispatchThreadID.y + 1 + round(Velocity.y * i) >= 0 && n3DispatchThreadID.y + 1 + round(Velocity.y * i) <= FRAME_HEIGHT &&
                Depth < VelocityMap[int2(n3DispatchThreadID.x + 1 + round(Velocity.x * i), n3DispatchThreadID.y + 1 + round(Velocity.y * i))].z)
            {
                cColor += RenderTarget[int2(n3DispatchThreadID.x + 1 + round(Velocity.x * i), n3DispatchThreadID.y + 1 + round(Velocity.y * i))];
                cnt++;
            }
        }
    }
    cColor /= cnt + 1;

    RWOutput[n3DispatchThreadID.xy] = cColor;
}
