#include "LightHelper.hlsl"

Texture2D g_Tex : register(t0);
Texture2D g_ShadowMap : register(t1);

SamplerState g_Samlinear : register(s0);

cbuffer VSConstantBufferEveryDrawing : register(b0) {
  matrix g_World;
  matrix g_WorldInvTranspose;
  Material g_Material;
};

cbuffer CBDrawingStates : register(b4) {
  int g_IsReflection;
  float3 pad;
}

cbuffer VSConstantBufferEveryFrame : register(b1) {
  matrix g_View;
  float4 g_EyePosW;
};

cbuffer VSConstantBufferOnResize : register(b2) {
  matrix g_Projection;
};

cbuffer PSConstantBuffer : register(b3) {
  matrix g_Reflection;
  DirectionalLight g_DirLight[10];
  PointLight g_PointLight[10];
  SpotLight g_SpotLight[10];
  int g_NumDirLight;
  int g_NumPointLight;
  int g_NumSpotLight;
  float g_bias;
  matrix g_DirLightMatrix;
}

// Input Type

// VertexPosNormalTex
// In
struct VertexPosNormalTex {
  float3 posL : POSITION;
  float3 normalL : NORMAL;
  float2 tex : TEXCOORD;
};
// Out
struct VertexPosHWNormalTex {
  float4 posH : SV_POSITION;
  float3 posW : POSITION;
  float3 normalW : NORMAL;
  float2 tex : TEXCOORD;
};

// VertexPosTex
// In
struct VertexPosTex {
  float3 posL : POSITION;
  float2 tex : TEXCOORD;
};

// Out
struct VertexPosHTex {
  float4 posH : SV_POSITION;
  float2 tex : TEXCOORD;
};
