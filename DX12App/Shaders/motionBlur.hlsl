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

[numthreads(32, 30, 1)]
void CS(int3 n3GroupThreadID : SV_GroupThreadID, int3 n3DispatchThreadID : SV_DispatchThreadID)
{
    int numSample = 10;

    float Depth = VelocityMap[int2(n3DispatchThreadID.x, n3DispatchThreadID.y)].b;

    int BlurConst = 50;

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


//float4 main(float2 texCoord : TEXCOORD0) : COLOR0
//{ // Calculate vector from pixel to light source in screen space.    
//    half2 deltaTexCoord = (texCoord - ScreenLightPos.xy);   
//    // Divide by number of samples and scale by control factor.   
//    deltaTexCoord *= 1.0f / NUM_SAMPLES * Density;  
//    // Store initial sample.   
//    half3 color = tex2D(frameSampler, texCoord);   
//    // Set up illumination decay factor.    
//    half illuminationDecay = 1.0f;   
//    // Evaluate summation from Equation 3 NUM_SAMPLES iterations.   
//    for (int i = 0; i < NUM_SAMPLES; i++)   
//    {     // Step sample location along ray.     
//        texCoord -= deltaTexCoord;     
//        // Retrieve sample at new location.    
//        half3 sample = tex2D(frameSampler, texCoord);     
//        // Apply sample attenuation scale/decay factors.     
//        sample *= illuminationDecay * Weight;     
//        // Accumulate combined color.     
//        color += sample;    
//        // Update exponential decay factor.     
//        illuminationDecay *= Decay;
//    }   
//    // Output final color with a further scale control factor.   
//    return float4( color * Exposure, 1); 
//} 