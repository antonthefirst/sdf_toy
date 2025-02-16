#include "show_texture_shared.inl"
#include "maths.inl"

uniform sampler2D tex;

// Interpolants
in  vec2 i_uv;

// Outputs
out vec4 o_color;

void main() {
    vec4 t = texture(tex, i_uv);

    o_color = t;
}
