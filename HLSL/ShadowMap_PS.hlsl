#include "ShadowMapBasic.hlsl"

float4 PS(float4 posH : SV_POSITION) : SV_TARGET
{
    float depth = posH.z / posH.w;
    return float4(depth,depth,depth,1.0f);
}
