#define NUM_LIGHTS 5

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
        
        else if (lights[i].Type == SPOT_LIGHT)
            result += shadowFactor * ComputeSpotLight(lights[i], mat, pos, normal, view);
        
        else if(lights[i].Type == POINT_LIGHT)
            result += shadowFactor * ComputePointLight(lights[i], mat, pos, normal, view);;
    }
    
    if (rimLightOn)
        result = result + CalcRimLight(0.5f, normal, view);
    
    return float4(result + mat.Emission, 0.0f);
}

