#include "Light.hlsli"

cbuffer MVPBuffer : register(b0)
{
    matrix gWorld;
    matrix gView;
    matrix gProjection;
    matrix gWorldInvTranspose;
};

VertexOut VS(VertexIn input)
{
    VertexOut o = (VertexOut)0;

    float4 WorldPos = mul(float4(input.position, 1.0f), gWorld);
    o.posW = WorldPos.xyz;
    float4 ViewPos = mul(WorldPos, gView);
    float4 ProjPos = mul(ViewPos, gProjection);
    o.position = ProjPos;

    o.color = input.color;

    float4 worldNormal = mul(float4(input.normal, 0.0f), gWorldInvTranspose);
    o.normal = normalize(worldNormal.xyz);

    return o;
}