#version 420 core
layout (std140, binding = 1) uniform Lights
{
                  // base  // offset
   vec3 l1_pos;   // 16    // 0
   vec3 l1_col;   // 16    // 16
   float l1_pow;  // 4     // 32
};

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in mat3 TBN;

struct Light
{
   vec3  pos;
   vec3  col;
   float power;
};

struct UVT
{
   vec2 uvOffset;
   vec2 uvScale;
};

uniform vec3 tint;
uniform Light light;
uniform vec3 ambient;
uniform vec3 camPos;
layout (binding = 0) uniform sampler2D base;
layout (binding = 1) uniform sampler2D normalMap;
uniform UVT uvt;

out vec4 FragColor;

void main()
{

   vec2 uv = TexCoord * uvt.uvScale + uvt.uvOffset;

   vec3 normal = texture(normalMap, uv).rgb;
   normal = normal * 2.0 - 1.0;
   normal = normalize(TBN * normal);

   vec3 lightDir =  normalize(l1_pos - FragPos);
   float normalFactor = max(dot(Normal, lightDir), 0.0);
   float factor = max(dot(normal, lightDir), 0.0) * l1_pow;
   
   vec3 diffuse = factor * l1_col;

   float specularStrength = 0.5;
   vec3 viewDir = normalize(camPos - FragPos);
   vec3 reflectDir = reflect(-lightDir, normal);
   float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128) * l1_pow;
   spec = spec * step(0.0, -step(normalFactor, 0.0));
   
   vec3 specular = specularStrength * spec * l1_col;

   FragColor = ((texture(base, uv) + vec4(tint, 1.0)) * vec4(diffuse + ambient + specular, 1.0));
}
