#include "Screen.hlsl"

// 像素着色器
float4 PS(VertexPosTexOut pIn) : SV_Target
{
    float4 color = g_Tex.Sample(g_Sampler, pIn.tex);
    return color;
}