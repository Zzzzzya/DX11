#include "LightHelper.hlsl"

cbuffer VSConstantBufferEveryDrawing : register(b0) {
  matrix g_World;
  matrix g_WorldInvTranspose;
  Material g_Material;
};

cbuffer LightBuffer : register(b1) {
  matrix g_View;
  matrix g_Projection;
};


// Input Type

// VertexPosNormalTex
// In
struct VertexPosNormalTex {
  float3 posL : POSITION;
  float3 normalL : NORMAL;
  float2 tex : TEXCOORD;
};

