#include "Light.hlsli"

cbuffer PSBuffer : register(b1)
{
        DirectionalLight dirLight;
        PointLight pointLight;
        SpotLight spotLight;
        Material material;
        float4 eyePos;
};

float4 PS(VertexOut input) : SV_TARGET
{
    DirectionalLight light = dirLight;
    PointLight pl = pointLight;
    Material mat = material;

    float3 normal = input.normal;
    float4 lightColor = 0.0f;

    float4 ambient = 0.0f;
    float4 specular = 0.0f;
    float4 diffuse = 0.0f;

    float4 A = 0.0f;
    float4 D = 0.0f;
    float4 S = 0.0f;

    float3 viewDir = normalize(eyePos - input.posW);

    // CalculateDirectionalLight(light, mat, normal,viewDir,A,D,S);

    // ambient += A;
    // diffuse += D;
    // specular += S;

    CalculatePointLight(pointLight, mat, normal, viewDir,input.posW, A, D, S);

    ambient += A;
    diffuse += D;
    specular += S;

    lightColor = input.color * (ambient + diffuse) + specular;

    return lightColor;
}