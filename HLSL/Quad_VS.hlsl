#include "Quad.hlsli"

VertexOut VS(VertexIn vin){
    VertexOut vout;
    vout.posH = mul(float4(vin.pos, 1.0f), World);
    vout.posH = mul(vout.posH, View);
    vout.posH = mul(vout.posH, Pro);
    vout.color = vin.color;
    return vout;
}