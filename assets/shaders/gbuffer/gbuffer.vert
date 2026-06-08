#version 460 core

out gl_PerVertex { vec4 gl_Position; };

struct VertexData 
{
  float position[3];
  float normal[3];
  float uv[2];
};

struct InstanceData 
{
  mat4 model;                // ← only the model matrix, no albedo
};

layout(std430, binding = 0) readonly buffer VertexBuffer     { VertexData vertex_data[]; };
layout(std430, binding = 1) readonly buffer CameraBuffer     { mat4 view; mat4 projection; };
layout(std430, binding = 2) readonly buffer InstanceBuffer   { InstanceData instance_data[]; };

layout(location = 0)      out vec2 out_uv;
layout(location = 1)      out vec3 out_normal;
layout(location = 2)      out vec3 out_world_pos;
layout(location = 3) flat out int  out_draw_id;

void main() 
{
  uint instance_id = gl_BaseInstance + gl_InstanceID;
  mat4 model       = instance_data[instance_id].model;

  vec3 pos = vec3(vertex_data[gl_VertexID].position[0],
                  vertex_data[gl_VertexID].position[1],
                  vertex_data[gl_VertexID].position[2]);
  vec3 normal = vec3(vertex_data[gl_VertexID].normal[0],
                     vertex_data[gl_VertexID].normal[1],
                     vertex_data[gl_VertexID].normal[2]);
  vec2 uv = vec2(vertex_data[gl_VertexID].uv[0],
                 vertex_data[gl_VertexID].uv[1]);

  vec4 world_pos    = model * vec4(pos, 1.0);
  vec3 world_normal = normalize(mat3(model) * normal);

  out_uv        = uv;
  out_normal    = world_normal;
  out_world_pos = world_pos.xyz;
  out_draw_id   = gl_DrawID;

  gl_Position = projection * view * world_pos;
}
