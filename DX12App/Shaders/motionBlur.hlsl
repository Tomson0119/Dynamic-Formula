///////////////////////////////////////////////////////////////////////////////
//
Texture2D VelocityMap : register(t0);
Texture2D RenderTarget : register(t1);

RWTexture2D<float4> RWOutput : register(u0);

groupshared float4 RenderTargetSharedCache[32 + 2][30 + 2];

[numthreads(32, 30, 1)]
void CS(int3 n3GroupThreadID : SV_GroupThreadID, int3 n3DispatchThreadID : SV_DispatchThreadID)
{
    int numSample = 10;
    float2 Velocity = VelocityMap[int2(n3DispatchThreadID.x, n3DispatchThreadID.y)].xy * 100;

    float4 cColor = RenderTarget[int2(n3DispatchThreadID.x + 1, n3DispatchThreadID.y + 1)];

    for (int i = 0; i < numSample; ++i)
    {
        cColor += RenderTarget[int2(n3DispatchThreadID.x + 1 + round(Velocity.x * i), n3DispatchThreadID.y + 1 + round(Velocity.y * i))];
    }
    
    cColor /= numSample + 1;

    RWOutput[n3DispatchThreadID.xy] = cColor;
}
