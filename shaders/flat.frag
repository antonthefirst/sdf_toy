#include "flat_shared.inl"
#include "maths.inl"

flat in vec4 v_col; 
     in vec2 v_uv;
     in vec3 v_nor_local;
     in vec3 v_pos_local;
flat in uint v_data;

out vec4 out_color;

void main()
{
	out_color = vec4(v_col);
}