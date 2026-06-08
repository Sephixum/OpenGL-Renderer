#version 460 core
#extension GL_ARB_bindless_texture : require

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec3 in_normal;
layout(location = 2) flat in int in_draw_id;

struct GPUMaterial 
{
  uvec2 albedo_handle;
  uvec2 normal_handle;
  uvec2 roughness_handle;
  uvec2 metallic_handle;
};

layout(std430, binding = 3) readonly buffer DrawMaterialIndicesBuffer 
{
  uint material_index_data[];
};

layout(std430, binding = 4) readonly buffer GPUMaterialBuffer 
{
  GPUMaterial material_data[];
};

layout(location = 0) out vec4 out_color;

// Lighting constants (example)
const vec3  light_dir        = normalize(vec3(1.0, 2.0, 1.0));
const vec3  light_color      = vec3(1.0);
const float ambient_strength = 0.15;

void main() 
{
  uint         material_index = material_index_data[in_draw_id];
  GPUMaterial  mat            = material_data[material_index];

  vec4 albedo = vec4(1.0);
  if (mat.albedo_handle != uvec2(0)) 
  {
    sampler2D s = sampler2D(mat.albedo_handle);
    albedo = texture(s, in_uv);
  }

  vec3  N       = normalize(in_normal);
  float diff    = max(dot(N, light_dir), 0.0);
  vec3  ambient = ambient_strength * light_color;
  vec3  diffuse = diff * light_color;
  vec3  result  = (ambient + diffuse) * albedo.rgb;

  out_color = vec4(result, albedo.a);
}
