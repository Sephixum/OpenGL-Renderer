#version 460 core
#extension GL_ARB_bindless_texture : require

layout(location = 0) in flat uvec2 in_albedo_bindless_handle;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;

const vec3 light_dir    = normalize(vec3(1.0, 2.0, 1.0));
const vec3 light_color  = vec3(1.0, 1.0, 1.0);

const float ambient_strength = 0.15;

layout(location = 0) out vec4 out_color;

void main()
{
  vec4 albedo = vec4(1.0);
  if (in_albedo_bindless_handle != uvec2(0))
  {
    sampler2D albedo_sampler = sampler2D(in_albedo_bindless_handle);
    albedo                   = texture(albedo_sampler, in_uv);
  }
  else
  {
    albedo = vec4(1.0);
  }
  vec3 normal = normalize(in_normal);

  // Diffuse: only positive, but that's okay – back faces get 0 here
  float diff    = max(dot(normal, light_dir), 0.0);
  vec3  diffuse = diff * light_color;

  // Ambient: constant low light
  vec3 ambient = ambient_strength * light_color;

  // Combine
  vec3 result = (ambient + diffuse) * albedo.rgb;
  out_color = vec4(result, 1.0);
}
