#include "Triangle.hlsli"  

float4 PS(VertexOut pIn):SV_TARGET{
    return pIn.color;
}