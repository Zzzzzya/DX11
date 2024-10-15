#include "Basic.hlsl"

VertexPosHWNormalTex VS(VertexPosNormalTex input)
{
    VertexPosHWNormalTex output;

    float4 posW = mul(float4(input.posL, 1.0f), g_World);
    
    float3 normalW = normalize(mul(input.normalL, (float3x3)g_WorldInvTranspose));

     // 若当前在绘制反射物体，先进行反射操作
     [flatten]
     if (g_IsReflection)
     {
         posW = mul(posW, g_Reflection);
         normalW = mul(normalW, (float3x3) g_Reflection);
     }

    float4 posV = mul(posW, g_View);
    float4 posH  = mul(posV,g_Projection);

    output.posV = posV;
    output.posW = posW.xyz;
    output.posH = posH;

    output.normalW = normalW;
    output.tex = input.tex;

    return output;
}
