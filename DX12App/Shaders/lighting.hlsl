#define NUM_LIGHTS 3

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
    float  padding0;
    float3 Position;
    float  padding1;
    float3 Direction;
    float  padding2;
    float  Range;
    int    Type;
};

float Pow5(float x)
{
    return (x * x * x * x * x);
}

float3 CalcReflectPercent(float3 fresnel, float3 h, float3 l)
{
    float oneMinusCosine = 1.0f - saturate(dot(h, l));
    return (fresnel + (1.0f - fresnel) * Pow5(oneMinusCosine));
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

float4 ComputeLighting(Light lights[NUM_LIGHTS], Material mat, 
                       float3 normal, float3 view, float shadowFactor[NUM_LIGHTS])
{
    float3 result = 0.0f;
    
    int i = 0;
    
    [unroll]
    for (i = 0; i < NUM_LIGHTS; ++i)
    {        
        if (lights[i].Type == DIRECTIONAL_LIGHT)
            result += ComputeDirectLight(lights[i], mat, normal, view);
        else if(lights[i].Type == SPOT_LIGHT)
            ;
        else
            ;
    }
    
    return float4(result + mat.Emission, 0.0f);
}

