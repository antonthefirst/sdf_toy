#include "visibility_shared.inl"
#include "maths.inl"

// Defines the functions to sample shapes SDF and other data.
#include "shapes_definition.inl"

// This depends on having ShapesSDF() defined
#include "shapes_intersect.inl"

// Interpolants
in  vec2 i_uv;

// Outputs
layout(location = 0) out vec3 o_col;
layout(location = 1) out vec3 o_nor;
layout(location = 2) out vec3 o_pos;

float getAspectRatioFromProjection() {
    // The aspect ratio can be extracted from our reverse-z projection matrix.
    return uni.proj_from_view[1][1] / uni.proj_from_view[0][0];
}

void main() {
    // Create a view space ray using the projection matrix.
    float aspect = getAspectRatioFromProjection();
    vec2 st = gl_FragCoord.xy;
    vec2 xy = xy_from_uv(i_uv) * vec2(aspect, 1);
    vec3 ray_ori_view = vec3(0.0);
    vec4 r = (uni.view_from_proj * vec4(xy.x/aspect, xy.y, 1.0, 1.0)); 
    vec3 ray_dir_view = normalize(r.xyz / r.w);

    // Transform into model space and trace
    vec3 ray_ori_model = (uni.model_from_view * vec4(ray_ori_view, 1.0)).xyz;
    vec3 ray_dir_model = (uni.model_from_view * vec4(ray_dir_view, 0.0)).xyz;
    float t = sphereTraceShapesSDF(ray_ori_model, ray_dir_model, 0.0, 100.0, 512);
  
    if (t > 0.0) {
        // On hit, write the surface data and depth.
        vec3 model_pos = ray_ori_model + ray_dir_model * t;
        //vec3 model_grad = gradient(model_pos);
        //vec3 model_nor = normalize(model_grad);
        vec4 proj_pos = uni.proj_from_model * vec4(model_pos, 1.0f);
        float z_ndc = proj_pos.z / proj_pos.w;
        float depth =  0.5 * z_ndc + 0.5;
        vec3 model_nor = ShapesNormal(model_pos);
        o_col = vec3(1);
        o_nor = (uni.view_from_model * vec4(model_nor, 0.0f)).xyz;
        o_pos = (uni.view_from_model * vec4(model_pos, 1.0f)).xyz;
        gl_FragDepth = depth;
    } else {
        // On miss, do nothing. (assume buffer is zeroed, and don't touch it if it isn't)
        //o_color = vec4(0);
        //gl_FragDepth = 0.0;
    }
}
