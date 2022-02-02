///////////////////////////////////////////////////////////////////////////////
//
Texture2D VelocityMap : register(t0);
Texture2D RenderTarget : register(t1);

RWTexture2D<float4> RWOutput : register(u0);

static float3 gf3ToLuminance = float3(0.33f, 0.33f, 0.33f);

#define _WITH_SOBEL_EDGE

#define _WITH_GROUPSHARED_MEMORY

groupshared float4 RenderTargetSharedCache[32 + 2][32 + 2];

void MotionBlur(int3 n3GroupThreadID : SV_GroupThreadID, int3 n3DispatchThreadID : SV_DispatchThreadID)
{
    float3 convolutionXMask = gf3ToLuminance;
    convolutionXMask.x -= VelocityMap[int2(n3DispatchThreadID.x, n3DispatchThreadID.y)].r * 2;
    convolutionXMask.z += VelocityMap[int2(n3DispatchThreadID.x, n3DispatchThreadID.y)].r * 2;

    float3 convolutionYMask = gf3ToLuminance;
    convolutionYMask.x -= VelocityMap[int2(n3DispatchThreadID.x, n3DispatchThreadID.y)].g * 2;
    convolutionYMask.z += VelocityMap[int2(n3DispatchThreadID.x, n3DispatchThreadID.y)].g * 2;

    float3 f3HorizontalEdge = (convolutionXMask.x * RenderTargetSharedCache[n3GroupThreadID.x][n3GroupThreadID.y + 1].rgb) + (convolutionXMask.y * RenderTargetSharedCache[n3GroupThreadID.x + 1][n3GroupThreadID.y + 1].rgb) + (convolutionXMask.z * RenderTargetSharedCache[n3GroupThreadID.x + 2][n3GroupThreadID.y + 1].rgb);
    float3 f3VerticalEdge = (convolutionYMask.x * RenderTargetSharedCache[n3GroupThreadID.x + 1][n3GroupThreadID.y].rgb) + (convolutionYMask.y * RenderTargetSharedCache[n3GroupThreadID.x + 1][n3GroupThreadID.y + 1].rgb) + (convolutionYMask.z * RenderTargetSharedCache[n3GroupThreadID.x + 1][n3GroupThreadID.y + 2].rgb);

    RWOutput[n3DispatchThreadID.xy] = float4(sqrt(f3HorizontalEdge * f3HorizontalEdge + f3VerticalEdge * f3VerticalEdge), 1.0f);

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
