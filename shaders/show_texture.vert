#include "show_texture_shared.inl"

// Interpolants
out vec2 i_uv; // texcoords are in the normalized [0,1] range for the viewport-filling quad part of the triangle

void main() {
        vec2 vertices[3]=vec2[3](vec2(-1,-1), vec2(3,-1), vec2(-1, 3));
        gl_Position = vec4(vertices[gl_VertexID],0,1);
        i_uv = 0.5 * gl_Position.xy + vec2(0.5);
}