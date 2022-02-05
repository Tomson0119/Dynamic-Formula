///////////////////////////////////////////////////////////////////////////////
//
Texture2D VelocityMap : register(t0);
Texture2D RenderTarget : register(t1);

RWTexture2D<float4> RWOutput : register(u0);

groupshared float4 RenderTargetSharedCache[32 + 2][30 + 2];

void MotionBlur(int3 n3GroupThreadID : SV_GroupThreadID, int3 n3DispatchThreadID : SV_DispatchThreadID)
{
    int numSample = 10;
    float2 Velocity = VelocityMap[int2(n3DispatchThreadID.x, n3DispatchThreadID.y)].xy * 100;

    float4 cColor = RenderTargetSharedCache[n3GroupThreadID.x + 1][n3GroupThreadID.y + 1];

    int cnt = 0;
    for (int i = 0; i < numSample; ++i)
    {
        if (n3GroupThreadID.x + 1 + floor(Velocity.x * i) >= 0 && n3GroupThreadID.x + 1 + floor(Velocity.x * i) < 32 + 2
             && n3GroupThreadID.y + 1 + floor(Velocity.y * i) >= 0 && n3GroupThreadID.y + 1 + floor(Velocity.y * i) < 30 + 2)
        {
            cColor += RenderTargetSharedCache[n3GroupThreadID.x + 1 + floor(Velocity.x * i)][n3GroupThreadID.y + 1 + floor(Velocity.y * i)];
            cnt++;
        }
    }
    
    cColor /= cnt + 1;

    RWOutput[n3DispatchThreadID.xy] = cColor;
}

[numthreads(32, 30, 1)]
void CS(int3 n3GroupThreadID : SV_GroupThreadID, int3 n3DispatchThreadID : SV_DispatchThreadID)
{
    RenderTargetSharedCache[n3GroupThreadID.x + 1][n3GroupThreadID.y + 1] = RenderTarget[int2(n3DispatchThreadID.x, n3DispatchThreadID.y)];

    if ((n3GroupThreadID.x == 0) && (n3GroupThreadID.y == 0))
        RenderTargetSharedCache[0][0] = RenderTarget[int2(n3DispatchThreadID.x - 1, n3DispatchThreadID.y - 1)];

    else if ((n3GroupThreadID.x == 31) && (n3GroupThreadID.y == 0))
        RenderTargetSharedCache[33][0] = RenderTarget[int2(n3DispatchThreadID.x + 1, n3DispatchThreadID.y - 1)];

    else if ((n3GroupThreadID.x == 0) && (n3GroupThreadID.y == 29))
        RenderTargetSharedCache[0][31] = RenderTarget[int2(n3DispatchThreadID.x - 1, n3DispatchThreadID.y + 1)];

    else if ((n3GroupThreadID.x == 31) && (n3GroupThreadID.y == 29))
        RenderTargetSharedCache[33][31] = RenderTarget[int2(n3DispatchThreadID.x + 1, n3DispatchThreadID.y + 1)];

    if (n3GroupThreadID.x == 0)
        RenderTargetSharedCache[0][n3GroupThreadID.y + 1] = RenderTarget[int2(n3DispatchThreadID.x - 1, n3DispatchThreadID.y)];

    if (n3GroupThreadID.y == 0)
        RenderTargetSharedCache[n3GroupThreadID.x + 1][0] = RenderTarget[int2(n3DispatchThreadID.x, n3DispatchThreadID.y - 1)];

    if (n3GroupThreadID.x == 31)    
        RenderTargetSharedCache[33][n3GroupThreadID.y + 1] = RenderTarget[int2(n3DispatchThreadID.x + 1, n3DispatchThreadID.y)];

    if (n3GroupThreadID.y == 29)
        RenderTargetSharedCache[n3GroupThreadID.x + 1][31] = RenderTarget[int2(n3DispatchThreadID.x, n3DispatchThreadID.y + 1)];

    GroupMemoryBarrierWithGroupSync();

    MotionBlur(n3GroupThreadID, n3DispatchThreadID);
}
