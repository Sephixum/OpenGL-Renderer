#version 460 core

layout(location = 0) in SHADER_BLOCK
{
  vec3 world_pos;
  vec3 normal;
  vec2 texcoord;
} 
f_in;

const vec3 lightDir = normalize(vec3(1.0, 2.0, 1.0));
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const vec3 objectColor = vec3(1.0, 1.0, 1.0);

const float ambientStrength = 0.15;

layout(location = 0) out vec4 out_color;

void main()
{
  vec3 normal = normalize(f_in.normal);
  
  // Diffuse: only positive, but that's okay – back faces get 0 here
  float diff    = max(dot(normal, lightDir), 0.0);
  vec3  diffuse = diff * lightColor;
  
  // Ambient: constant low light
  vec3 ambient = ambientStrength * lightColor;
  
  // Combine
  vec3 result = (ambient + diffuse) * objectColor;
  out_color = vec4(result, 1.0);
}
