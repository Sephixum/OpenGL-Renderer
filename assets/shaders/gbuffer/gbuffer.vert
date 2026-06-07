#version 460 core

out gl_PerVertex 
{
  vec4 gl_Position;
};

struct VertexData
{
  float position[3];
  float normal[3];
  float uv[2];
};

struct InstanceData
{
  mat4  model;
  uvec2 albedo_handle;
};


layout(std430, binding = 0) readonly buffer VertexBuffer
{
  VertexData vertex_data[];
};

layout(std430, binding = 1) readonly buffer CameraBuffer
{
  mat4 view;
  mat4 projection;
};

layout(std430, binding = 2) readonly buffer InstanceBuffer
{
  InstanceData instance_data[];
};


vec3 GetPosition(uint index)
{
  return vec3(
    vertex_data[index].position[0],
    vertex_data[index].position[1],
    vertex_data[index].position[2]
  );
}

mat4 GetInstanceModelMatrix()
{
  return instance_data[(gl_BaseInstance + gl_InstanceID)].model;
}

uvec2 GetInstanceAlbedoHandle()
{
  return instance_data[(gl_BaseInstance + gl_InstanceID)].albedo_handle;
;
}

vec3 GetNormal(uint index)
{
  return vec3(
    vertex_data[index].normal[0],
    vertex_data[index].normal[1],
    vertex_data[index].normal[2]
  );
}

vec2 GetUV(uint index)
{
  return vec2(
    vertex_data[index].uv[0],
    vertex_data[index].uv[1]
  );
}

layout(location = 0) out flat uvec2 out_albedo_bindless_handle;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec3 out_normal;

void main()
{
  mat4 model        = GetInstanceModelMatrix();
  vec4 world_pos    = model * vec4(GetPosition(gl_VertexID), 1.0);
  mat3 normal_mat   = mat3(model);
  vec3 world_normal = normalize(normal_mat * GetNormal(gl_VertexID));

  out_uv     = GetUV(gl_VertexID);
  out_normal = GetNormal(gl_VertexID);
  out_albedo_bindless_handle = GetInstanceAlbedoHandle();

  gl_Position = projection * view * world_pos;
}
