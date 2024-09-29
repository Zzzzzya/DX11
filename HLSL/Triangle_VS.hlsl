#include "Triangle.hlsli"

VertexOut VS(VertexIn vin){
    VertexOut vout;
    vout.posH = float4(vin.pos,1.0f);
    vout.color = vin.color;
    return vout;
}