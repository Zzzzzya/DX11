#include "ShadowMapBasic.hlsl"

float4 VS(VertexPosNormalTex input) : SV_POSITION
{
    float4 posW = mul(float4(input.posL, 1.0f), g_World);
    float4 posV = mul(posW, g_View);
    float4 posH  = mul(posV,g_Projection);
    return posH;
}