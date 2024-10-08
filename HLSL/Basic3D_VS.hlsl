#include "Basic.hlsl"

VertexPosHWNormalTex VS(VertexPosNormalTex input)
{
    VertexPosHWNormalTex output;

    float4 posW = mul(float4(input.posL, 1.0f), g_World);
    float4 posV = mul(posW, g_View);
    float4 posH  = mul(posV,g_Projection);

    output.posW = posW.xyz;
    output.posH = posH;

    output.normalW = normalize(mul(input.normalL, (float3x3)g_WorldInvTranspose));
    output.tex = input.tex;

    return output;
}
