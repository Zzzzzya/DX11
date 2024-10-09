// Material
struct Material {
  float4 ambient;
  float4 diffuse;
  float4 specular; // w = 镜面反射强度
  float4 reflect;
}; // 16 * 4 = 64 bytes

// Lights

struct DirectionalLight {
  float4 ambient;
  float4 diffuse;
  float4 specular;
  float3 direction;
  float pad;
}; // 16 * 4 = 64 bytes

// 点光
struct PointLight {
  float4 ambient;
  float4 diffuse;
  float4 specular;

  float3 position;
  float range;

  float3 att;
  float pad;
}; // 16 * 4 = 64 bytes

// 聚光灯
struct SpotLight {
  float4 ambient;
  float4 diffuse;
  float4 specular;

  float3 position;
  float range;

  float3 direction;
  float Spot;

  float3 att;
  float pad;
}; // 16 * 4 = 64 bytes

float4 CalculateDirectionalLight(DirectionalLight light, Material mat,
                                 float3 normal, float3 viewDir, out float4 A,
                                 out float4 D, out float4 S) {
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

void CalculatePointLight(PointLight light, Material mat, float3 normal,
                           float3 viewDir, float3 pos, out float4 A,
                           out float4 D, out float4 S) {
  A = float4(0.0, 0.0f, 0.0f, 0.0f);
  D = float4(0.0, 0.0f, 0.0f, 0.0f);
  S = float4(0.0, 0.0f, 0.0f, 0.0f);

  float3 lightVec = light.position - pos;
  float d = length(lightVec);

  lightVec /= d;

  A = mat.ambient * light.ambient;

  float diffuseFactor = dot(lightVec, normal);

  [flatten] if (diffuseFactor > 0.0f) {
    float3 v = reflect(-lightVec, normal);
    float specFactor = pow(saturate(dot(v, viewDir)), mat.specular.w);

    D = diffuseFactor * mat.diffuse * light.diffuse;
    S = specFactor * mat.specular * light.specular;
  }

  float att =
      1.0f / dot(light.att, float3(1.0f, d, d * d));

  D *= att;
  S *= att;
}

void CalculateSpotLight(SpotLight light, Material mat, float3 normal,
                        float3 viewDir, float3 pos, out float4 A, out float4 D,
                        out float4 S) {

  A = float4(0.0, 0.0f, 0.0f, 0.0f);
  D = float4(0.0, 0.0f, 0.0f, 0.0f);
  S = float4(0.0, 0.0f, 0.0f, 0.0f);

  float3 lightVec = light.position - pos;
  float d = length(lightVec);

  lightVec /= d;

  A = mat.ambient * light.ambient;

  float diffuseFactor = dot(lightVec, normal);

  [flatten] if (diffuseFactor > 0.0f) {
    float3 v = reflect(-lightVec, normal);
    float specFactor = pow(saturate(dot(v, viewDir)), mat.specular.w);

    D = diffuseFactor * mat.diffuse * light.diffuse;
    S = specFactor * mat.specular * light.specular;
  }

  float spot = pow(max(dot(lightVec, -light.direction), 0.0f), light.Spot);
  float att = spot / dot(light.att, float3(1.0f, d, d * d));

  A *= spot;
  D *= spot;
  S *= spot;
}