#include "Quad.hlsli"

float4 PS(VertexOut pin) : SV_TARGET {
    return pin.color;
}