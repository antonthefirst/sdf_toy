#include "flat_shared.inl"
//uniform mat4 proj_from_model;
//uniform mat4 view_from_model;
//uniform float depth_offset;

// Inputs
in vec3 in_pos_local;
in vec3 in_nor_local;
in vec4 in_col;
in vec2 in_uv;
in uint in_data;

// Interpolants
out vec3 v_nor_local;
flat out vec4 v_col;
out vec2 v_uv;
out vec3 v_pos_local;
flat out uint v_data;

void main(){
  vec4 pos_clip = uni.proj_from_model * vec4(in_pos_local,1);
  pos_clip.z += uni.depth_offset;
  gl_Position = pos_clip;
  v_col = in_col;
  v_uv  = in_uv;
  v_nor_local = in_nor_local;
  v_pos_local = in_pos_local;
  v_data = in_data;
}