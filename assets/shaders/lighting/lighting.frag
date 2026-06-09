#version 460 core
#extension GL_ARB_bindless_texture : require

in vec2 in_uv;
out vec4 out_color;

// G‑Buffer handles – four uvec2 values in an SSBO at binding 5
struct GBufferHandles
{
    uvec2 albedo_handle;
    uvec2 normal_handle;
    uvec2 position_handle;
    uvec2 material_handle;   // available for future PBR, not used yet
};

layout(std430, binding = 5) readonly buffer GBufferHandleBuffer
{
    GBufferHandles handles;
};

// Lighting constants – same as your forward shader
const vec3  light_dir        = normalize(vec3(1.0, 2.0, 1.0));
const vec3  light_color      = vec3(1.0);
const float ambient_strength = 0.15;

void main()
{
  // Re‑interpret the handles as samplers
  sampler2D albedo_sampler   = sampler2D(handles.albedo_handle);
  sampler2D normal_sampler   = sampler2D(handles.normal_handle);
  sampler2D position_sampler = sampler2D(handles.position_handle);

  // Sample G‑Buffer
  vec4 albedo    = texture(albedo_sampler,   in_uv);
  vec3 N         = texture(normal_sampler,   in_uv).rgb * 2.0 - 1.0;   // decode
  vec3 world_pos = texture(position_sampler, in_uv).rgb;

  // Simple directional light
  float diff   = max(dot(normalize(N), light_dir), 0.0);
  vec3 ambient = ambient_strength * light_color;
  vec3 diffuse = diff * light_color;
  vec3 result  = (ambient + diffuse) * albedo.rgb;

  out_color = vec4(result, albedo.a);
}
