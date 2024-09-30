cbuffer ConstantBuffer : register(b0) {
  matrix World;
  matrix View;
  matrix Pro;
}

struct VertexIn {
  float3 pos : POSITION;
  float4 color : COLOR;
};

struct VertexOut {
  float4 posH : SV_POSITION;
  float4 color : COLOR;
};