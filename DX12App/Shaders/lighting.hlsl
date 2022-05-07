#define NUM_LIGHTS 16

#define POINT_LIGHT       1
#define SPOT_LIGHT        2
#define DIRECTIONAL_LIGHT 3

struct Material
{
    float3 Ambient;
    float  padding0;
    float4 Diffuse;
    float3 Specular;
    float  Exponent;
    float3 Emission;
    float  IOR;
};

struct Light
{
    float3 Diffuse;
    float  FalloffStart;
    float3 Position;
    float  FalloffEnd;
    float3 Direction;
    float  SpotPower;
    float  Range;
    int    Type;
};

float Pow5(float x)
{
    return (x * x * x * x * x);
}

float DoAttenuation(float distance, float range)
{
    float att = saturate(1.0f - (distance * distance / (range * range)));
    return att * att;
}

static const float BayerMatrix8[8][8] =
{
    { 1.0 / 65.0, 49.0 / 65.0, 13.0 / 65.0, 61.0 / 65.0, 4.0 / 65.0, 52.0 / 65.0, 16.0 / 65.0, 64.0 / 65.0 },
    { 33.0 / 65.0, 17.0 / 65.0, 45.0 / 65.0, 29.0 / 65.0, 36.0 / 65.0, 20.0 / 65.0, 48.0 / 65.0, 32.0 / 65.0 },
    { 9.0 / 65.0, 57.0 / 65.0, 5.0 / 65.0, 53.0 / 65.0, 12.0 / 65.0, 60.0 / 65.0, 8.0 / 65.0, 56.0 / 65.0 },
    { 41.0 / 65.0, 25.0 / 65.0, 37.0 / 65.0, 21.0 / 65.0, 44.0 / 65.0, 28.0 / 65.0, 40.0 / 65.0, 24.0 / 65.0 },
    { 3.0 / 65.0, 51.0 / 65.0, 15.0 / 65.0, 63.0 / 65.0, 2.0 / 65.0, 50.0 / 65.0, 14.0 / 65.0, 62.0 / 65.0 },
    { 35.0 / 65.0, 19.0 / 65.0, 47.0 / 65.0, 31.0 / 65.0, 34.0 / 65.0, 18.0 / 65.0, 46.0 / 65.0, 30.0 / 65.0 },
    { 11.0 / 65.0, 59.0 / 65.0, 7.0 / 65.0, 55.0 / 65.0, 10.0 / 65.0, 58.0 / 65.0, 6.0 / 65.0, 54.0 / 65.0 },
    { 43.0 / 65.0, 27.0 / 65.0, 39.0 / 65.0, 23.0 / 65.0, 42.0 / 65.0, 26.0 / 65.0, 38.0 / 65.0, 22.0 / 65.0 }
};


float2 dither(float2 coord, float seed, float2 size)
{
    float noiseX = ((frac(1.0 - (coord.x + seed * 1.0) * (size.x / 2.0)) * 0.25) + (frac((coord.y + seed * 2.0) * (size.y / 2.0)) * 0.75)) * 2.0 - 1.0;
    float noiseY = ((frac(1.0 - (coord.x + seed * 3.0) * (size.x / 2.0)) * 0.75) + (frac((coord.y + seed * 4.0) * (size.y / 2.0)) * 0.25)) * 2.0 - 1.0;
    return float2(noiseX, noiseY);
}

inline float ditherMask8(in float2 pixel)
{
    return BayerMatrix8[pixel.x % 8][pixel.y % 8];
}

inline float dither(in float2 pixel)
{
    return ditherMask8(pixel);
}

float3 CalcReflectPercent(float3 fresnel, float3 h, float3 l)
{
    float oneMinusCosine = 1.0f - saturate(dot(h, l));
    return (fresnel + (1.0f - fresnel) * Pow5(oneMinusCosine));
}

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

float CalcRimLight(float rimWidth, float3 normal, float3 view)
{
    return smoothstep(1.0f - rimWidth, 1.0f, 1 - max(0, dot(normal, view)));
}

float3 BlinnPhongModelLighting(float3 lightDiff, float3 lightVec, float3 normal, float3 view, Material mat)
{
    const float m = mat.Exponent;
    const float f = ((mat.IOR - 1) * (mat.IOR - 1)) / ((mat.IOR + 1) * (mat.IOR + 1));
    const float3 fresnel0 = float3(f, f, f);
    
    float3 halfVec = normalize(view + lightVec);
    
    float roughness = (m + 8.0f) * pow(saturate(dot(halfVec, normal)), m) / 8.0f;
    float3 fresnel = CalcReflectPercent(fresnel0, halfVec, lightVec);
    float3 specular = fresnel * roughness;
    specular = specular / (specular + 1.0f);
    
    return (mat.Diffuse.rgb + specular * mat.Specular) * lightDiff;
}

float3 ComputeDirectLight(Light light, Material mat, float3 normal, float3 view)
{
    float3 lightVec = normalize(light.Direction);
    
    float irradiance = max(dot(normal, lightVec), 0.0f);
    float3 lightDiffuse = light.Diffuse * irradiance;
    
    return BlinnPhongModelLighting(lightDiffuse, lightVec, normal, view, mat);
}

float3 ComputePointLight(Light light, Material mat, float3 pos, float3 normal, float3 view)
{
    float3 result = float3(0.0f, 0.0f, 0.0f);
    bool bCompute = true;
    
    float3 lightVec = light.Position - pos;
    
    float d = length(lightVec);
    
    if (d > light.FalloffEnd)
        bCompute = false;
    
    
    if (bCompute)
    {
        lightVec /= d;
    
        float ndotl = max(dot(lightVec, normal), 0.0f);
        float3 lightDiffuse = light.Diffuse * ndotl;
    
        float att = CalcAttenuation(d, light.FalloffStart, light.FalloffEnd);
        lightDiffuse *= att;

        result = BlinnPhongModelLighting(lightDiffuse, lightVec, normal, view, mat);
    }
    
    return result;
}

float3 ComputeSpotLight(Light light, Material mat, float3 pos, float3 normal, float3 view)
{
    float3 result = float3(0.0f, 0.0f, 0.0f);
    bool bCompute = true;
    
    float3 lightVec = light.Position - pos;
    
    float d = length(lightVec);
    
    if (d > light.FalloffEnd)
        bCompute = false;
    
    if (bCompute)
    {
        lightVec /= d;
    
        float ndotl = max(dot(lightVec, normal), 0.0f);
        float3 lightDiffuse = light.Diffuse * ndotl;
    
        float att = CalcAttenuation(d, light.FalloffStart, light.FalloffEnd);
        lightDiffuse *= att;
    
        float spotFactor = pow(max(dot(-lightVec, light.Direction), 0.0f), light.SpotPower);
        lightDiffuse *= spotFactor;

        result = BlinnPhongModelLighting(lightDiffuse, lightVec, normal, view, mat);
    }
    
    return result;
}

float4 ComputeLighting(Light lights[NUM_LIGHTS], Material mat, float3 pos, float3 normal, float3 view, float shadowFactor, bool rimLightOn)
{
    float3 result = 0.0f;
    
    int i = 0;
    
    [unroll]
    for (i = 0; i < NUM_LIGHTS; ++i)
    {        
        if (lights[i].Type == DIRECTIONAL_LIGHT)
            result += shadowFactor * ComputeDirectLight(lights[i], mat, normal, view);
        
        //else if (lights[i].Type == SPOT_LIGHT)
            //result += ComputeSpotLight(lights[i], mat, pos, normal, view);
        
        else if(lights[i].Type == POINT_LIGHT)
            result += ComputePointLight(lights[i], mat, pos, normal, view);;
    }
    
    if (rimLightOn)
        result = result + CalcRimLight(0.5f, normal, view);
    
    return float4(result + mat.Emission, 0.0f);
}

