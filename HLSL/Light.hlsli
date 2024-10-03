#include "LightHelper.hlsli"

struct VertexIn
{
  float3 position : POSITION;
  float3 normal : NORMAL;
  float4 color : COLOR;
};

struct VertexOut
{
  float4 position : SV_POSITION;
  float3 posW : POSITION;
  float4 color : COLOR;
  float3 normal : NORMAL;
};

