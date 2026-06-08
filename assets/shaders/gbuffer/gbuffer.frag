#version 460 core
#extension GL_ARB_bindless_texture : require

layout(location = 0)      in vec2 in_uv;
layout(location = 1)      in vec3 in_normal;
layout(location = 2)      in vec3 in_world_pos;
layout(location = 3) flat in int  in_draw_id;

layout(location = 0) out vec4 out_albedo;
layout(location = 1) out vec4 out_normal;
layout(location = 2) out vec4 out_position;
layout(location = 3) out vec4 out_material;


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

void main() 
{
  uint         material_index = material_index_data[in_draw_id];
  GPUMaterial  mat            = material_data[material_index];

  vec4 albedo = vec4(1.0);
  if (mat.albedo_handle != uvec2(0)) 
  {
    sampler2D s = sampler2D(mat.albedo_handle);
    albedo      = texture(s, in_uv);
  }

  float roughness = 0.5;
  if (mat.roughness_handle != uvec2(0)) 
  {
    sampler2D s = sampler2D(mat.roughness_handle);
    roughness   = texture(s, in_uv).g;
  }

  float metallic = 0.0;   // default
  if (mat.metallic_handle != uvec2(0)) 
  {
    sampler2D s = sampler2D(mat.metallic_handle);
    metallic    = texture(s, in_uv).b;
  }

  float ao = 1.0;

  vec3 calculated_normal = normalize(in_normal);

  out_albedo   = albedo;
  out_normal   = vec4(calculated_normal * 0.5 + 0.5, 1.0);
  out_material = vec4(roughness, metallic, ao, 1.0);
  out_position = vec4(in_world_pos, 1.0);
}
