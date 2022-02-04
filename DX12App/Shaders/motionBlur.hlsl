///////////////////////////////////////////////////////////////////////////////
//
Texture2D VelocityMap : register(t0);
Texture2D RenderTarget : register(t1);

SamplerState gLinearWrap : register(s0);

RWTexture2D<float4> RWOutput : register(u0);

static float3 gf3ToLuminance = float3(0.33f, 0.33f, 0.33f);

groupshared float4 RenderTargetSharedCache[32 + 2][32 + 2];

void MotionBlur(int3 n3GroupThreadID : SV_GroupThreadID, int3 n3DispatchThreadID : SV_DispatchThreadID)
{
    int numSample = 10;
    float2 Velocity = VelocityMap[int2(n3DispatchThreadID.x, n3DispatchThreadID.y)].xy;
    float2 uv;

    uv.x = n3DispatchThreadID.x / 1920;
    uv.y = n3DispatchThreadID.y / 1080;

    float4 cColor = RenderTargetSharedCache[n3GroupThreadID.x + 1][n3GroupThreadID.y + 1];

    for (int i = 0; i < numSample; ++i)
    {
        Velocity.xy;
        cColor += RenderTarget.SampleLevel(gLinearWrap, uv + Velocity * i, 0);
    }
    
    cColor /= numSample;

    RWOutput[n3DispatchThreadID.xy] = cColor;
}

[numthreads(32, 32, 1)]
void CS(int3 n3GroupThreadID : SV_GroupThreadID, int3 n3DispatchThreadID : SV_DispatchThreadID)
{
    RenderTargetSharedCache[n3GroupThreadID.x + 1][n3GroupThreadID.y + 1] = RenderTarget[int2(n3DispatchThreadID.x, n3DispatchThreadID.y)];

    if ((n3GroupThreadID.x == 0) && (n3GroupThreadID.y == 0))
        RenderTargetSharedCache[0][0] = RenderTarget[int2(n3DispatchThreadID.x - 1, n3DispatchThreadID.y - 1)];

    else if ((n3GroupThreadID.x == 31) && (n3GroupThreadID.y == 0))
        RenderTargetSharedCache[33][0] = RenderTarget[int2(n3DispatchThreadID.x + 1, n3DispatchThreadID.y - 1)];

    else if ((n3GroupThreadID.x == 0) && (n3GroupThreadID.y == 31))
        RenderTargetSharedCache[0][33] = RenderTarget[int2(n3DispatchThreadID.x - 1, n3DispatchThreadID.y + 1)];

    else if ((n3GroupThreadID.x == 31) && (n3GroupThreadID.y == 31))
        RenderTargetSharedCache[33][33] = RenderTarget[int2(n3DispatchThreadID.x + 1, n3DispatchThreadID.y + 1)];

    if (n3GroupThreadID.x == 0)
        RenderTargetSharedCache[0][n3GroupThreadID.y + 1] = RenderTarget[int2(n3DispatchThreadID.x - 1, n3DispatchThreadID.y)];

    if (n3GroupThreadID.y == 0)
        RenderTargetSharedCache[n3GroupThreadID.x + 1][0] = RenderTarget[int2(n3DispatchThreadID.x, n3DispatchThreadID.y - 1)];

    if (n3GroupThreadID.x == 31)
        RenderTargetSharedCache[33][n3GroupThreadID.y + 1] = RenderTarget[int2(n3DispatchThreadID.x + 1, n3DispatchThreadID.y)];

    if (n3GroupThreadID.y == 31)
        RenderTargetSharedCache[n3GroupThreadID.x + 1][33] = RenderTarget[int2(n3DispatchThreadID.x, n3DispatchThreadID.y + 1)];

    GroupMemoryBarrierWithGroupSync();

    MotionBlur(n3GroupThreadID, n3DispatchThreadID);
}
