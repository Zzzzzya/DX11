// Material
struct Material
{
  float4 ambient;
  float4 diffuse;
  float4 specular; // w = 镜面反射强度
  float4 reflect;
};

// Lights

struct DirectionalLight
{
  float4 ambient;
  float4 diffuse;
  float4 specular;
  float3 direction;
  float pad;
};

// 点光
struct PointLight
{
  float4 ambient;
  float4 diffuse;
  float4 specular;

  float3 position;
  float range;

  float3 att;
  float pad;
};

// 聚光灯
struct SpotLight
{
  float4 ambient;
  float4 diffuse;
  float4 specular;

  float3 position;
  float range;

  float3 direction;
  float Spot;

  float3 att;
  float pad;
};

float4 CalculateDirectionalLight(DirectionalLight light, Material mat,
                                 float3 normal, float3 viewDir, out float4 A,
                                 out float4 D, out float4 S)
{
  float3 lightDir = -light.direction;
  float4 FinalColor = 0.0f;

  // Ambient
  float4 ambient = mat.ambient * light.ambient;

  // Diffuse
  float4 diff = saturate(dot(normal, lightDir));
  float4 diffuse = diff * mat.diffuse * light.diffuse;

  // Specular
  float3 h = normalize(lightDir + viewDir);
  float spec = pow(saturate(dot(normal, h)), mat.specular.w);
  float4 specular = spec * mat.specular * light.specular;

  A = ambient;
  D = diffuse;
  S = specular;
  FinalColor = ambient + diffuse + specular;

  return FinalColor;
}

float4 CalculatePointLight(PointLight light, Material mat, float3 normal,
                           float3 viewDir, float3 pos, out float4 A,
                           out float4 D, out float4 S)
{
  float3 lightDir = light.position - pos;
  float distance = length(lightDir);
  lightDir /= distance;

  float4 FinalColor = 0.0f;

  // Ambient
  float4 ambient = mat.ambient * light.ambient;

  // Diffuse
  float4 diff = saturate(dot(normal, lightDir));
  float4 diffuse = diff * mat.diffuse * light.diffuse;

  // Specular
  float3 h = normalize(lightDir + viewDir);
  float spec = pow(saturate(dot(normal, h)), mat.specular.w);
  float4 specular = spec * mat.specular * light.specular;

  float att =
      1.0f / dot(light.att, float3(1.0f, distance, distance * distance));

  diffuse *= att;
  spec *= att;

  A = ambient;
  D = diffuse;
  S = specular;

  FinalColor = ambient + diffuse + specular;

  return FinalColor;
}