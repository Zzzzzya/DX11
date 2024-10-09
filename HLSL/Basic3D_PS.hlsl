#include "Basic.hlsl"

float4 PS(VertexPosHWNormalTex pIn) : SV_TARGET
{
    float4 texColor = g_Tex.Sample(g_Samlinear, pIn.tex);
    clip(texColor.a - 0.1f);


    // Normal
    pIn.normalW = normalize(pIn.normalW);

    // View ( Point -> Eye)
    float3 View = normalize(g_EyePosW.xyz - pIn.posW);

    // 初始化0
    float4 ambient = float4(0.0, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0, 0.0f, 0.0f, 0.0f);
    float4 specular = float4(0.0, 0.0f, 0.0f, 0.0f);
    float4 A = float4(0.0, 0.0f, 0.0f, 0.0f);
    float4 D = float4(0.0, 0.0f, 0.0f, 0.0f);
    float4 S = float4(0.0, 0.0f, 0.0f, 0.0f);
    int i;

    for (i=0;i<g_NumDirLight;i++)
    {
        CalculateDirectionalLight(g_DirLight[i], g_Material, pIn.normalW, View, A, D, S);
        ambient += A;
        diffuse += D;
        specular += S;
    }

    PointLight pointlight;
    for (i=0;i<g_NumPointLight;i++)
    {
        pointlight = g_PointLight[i];   
        [flatten]
        if (g_IsReflection)
        {
            pointlight.position = (float3) mul(float4(pointlight.position, 1.0f), g_Reflection);
        }
        CalculatePointLight(pointlight, g_Material, pIn.normalW, View, pIn.posW, A, D, S);
        ambient += A;
        diffuse += D;
        specular += S;
    }
    
    for(i=0;i<g_NumSpotLight;i++)
    {
        CalculateSpotLight(g_SpotLight[i], g_Material, pIn.normalW, View, pIn.posW, A, D, S);
        ambient += A;
        diffuse += D;
        specular += S;
    }


    
    float4 litColor = texColor * (ambient + diffuse) + specular;
    litColor.a = texColor.a * g_Material.diffuse.a;

    return litColor;
}
