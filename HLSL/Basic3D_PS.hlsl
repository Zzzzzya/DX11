#include "Basic.hlsl"

float4 PS(VertexPosHWNormalTex pIn) : SV_TARGET
{
    // Normal
    pIn.normalW = normalize(pIn.normalW);

    // View ( Point -> Eye)
    float View = normalize(g_EyePosW - pIn.posW);

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

    for (i=0;i<g_NumPointLight;i++)
    {
        CalculatePointLight(g_PointLight[i], g_Material, pIn.normalW, View, pIn.posW, A, D, S);
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


    float4 texColor = g_Tex.Sample(g_Samlinear, pIn.tex);
    float4 litColor = texColor * (ambient + diffuse) + specular;
    // litColor.a = texColor.a * g_Material.diffuse.a;

    return litColor;
}
