Texture2D g_Tex : register(t0); // Screen texture
SamplerState g_Sampler : register(s0); // Screen sampler

struct VertexPosTex{
    float3 posL : POSITION;
    float2 tex : TEXCOORD;
};

struct VertexPosTexOut{
    float4 posH : SV_POSITION;
    float2 tex : TEXCOORD;
};