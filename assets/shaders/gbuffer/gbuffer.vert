#version 460 core

struct VertexData
{
  float position[3];
  float normal[3];
  float uv[2];
};

struct InstanceData
{
  mat4 model;
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


vec3 get_position(uint index)
{
  return vec3(
    vertex_data[index].position[0],
    vertex_data[index].position[1],
    vertex_data[index].position[2]
  );
}

vec3 get_normal(uint index)
{
  return vec3(
    vertex_data[index].normal[0],
    vertex_data[index].normal[1],
    vertex_data[index].normal[2]
  );
}

vec2 get_uv(uint index)
{
  return vec2(
    vertex_data[index].uv[0],
    vertex_data[index].uv[1]
  );
}

layout(location = 0) out SHADER_BLOCK
{
  vec3 world_pos;
  vec3 normal;
  vec2 texcoord;
} 
v_out;

void main()
{
  mat4 model        = instance_data[gl_InstanceID].model;
  vec4 world_pos    = model * vec4(get_position(gl_VertexID), 1.0);
  mat3 normal_mat   = mat3(model);
  vec3 world_normal = normalize(normal_mat * get_normal(gl_VertexID));

  v_out.world_pos = world_pos.xyz;
  v_out.normal    = world_normal;
  v_out.texcoord  = get_uv(gl_VertexID);

  gl_Position = projection * view * world_pos;
}
