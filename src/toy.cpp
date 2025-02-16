#include "wrap/app_wrap.h"
#include "wrap/input_wrap.h"
#include "imgui/imgui.h"
#include "wrap/font_wrap.h"
#include "math/maths.h"
#include "math/pose3.h"
#include "math/mat4.h"
#include "core/colorspace.h"
#include "core/mesh_maker.h"
#include "core/color.h"
#include "core/times.h"
#include "core/dir.h"
#include "core/cpu_timer.h"
#include "shaders/rngeasy.inl"

#include "saveload.h"
#include "control.h"
#include "render.h"
#include "orbit_camera.h"
#include "draw.h"

// Shared shapes definition and tracing
#include "shaders/shapes_definition.inl"
#include "shaders/shapes_intersect.inl"

float getAspectRatioFromProjection(const mat4& proj_from_view) {
    // The aspect ratio can be extracted from our reverse-z projection matrix.
    return proj_from_view[5] / proj_from_view[0];
}
bool intersectShapes(vec2 mouse_unorm, const mat4& proj_from_view, const mat4& model_from_view, const mat4& world_from_model, vec3* world_pos) {
    // Create a view space ray using the projection matrix.
    float aspect = getAspectRatioFromProjection(proj_from_view);
    vec2 xy = xy_from_uv(mouse_unorm) * vec2(aspect, 1);
    vec3 ray_ori_view = vec3(0.0);
    vec4 r = xfmVec(inverse(proj_from_view), vec4(xy.x/aspect, xy.y, 1.0, 1.0)); 
    vec3 ray_dir_view = normalize(r.xyz() / r.w);

    // Transform into model space and trace
    vec3 ray_ori_model = xfmVec(model_from_view, vec4(ray_ori_view, 1.0)).xyz();
    vec3 ray_dir_model = xfmVec(model_from_view, vec4(ray_dir_view, 0.0)).xyz();
    float t = sphereTraceShapesSDF(ray_ori_model, ray_dir_model, 0.0, 100.0, 512);
    if (t > 0.0f) {
        vec3 model_pos = ray_ori_model + ray_dir_model * t;
        *world_pos = xfmVec(world_from_model, vec4(model_pos, 1.0f)).xyz();
        return true;
    } else {
        return false;
    }
}

// For raw OpenGL calls. Should probably wrap these.
#include <GL/gl3w.h>

// Globals
Input           g_hwin;
GeneralDrawer   g_Draw;

static OrbitCamera s_camera;

void toyInit()
{
	{
		BlockTimer timer("RenderInit");
	}
    /*
	if (!g_font) {
		BlockTimer timer("FontInit");
		g_font = fontMake(g_font_file, g_font_scale);
	}
    */
}
void toyTerm()
{

}

bool toyLoop(Input& _in, AppState& app)
{
	g_hwin = _in;
	g_In.newFrame();

	ivec2 res = ivec2(app.res_x, app.res_y);
	//float aspect = float(res.x) / float(res.y);

    // Clear the screen
    vec3 clear_col = vec3(0.25f, 0.25f, 0.25f); 
    glViewport(0, 0, res.x, res.y);
    glClearColor(clear_col.x, clear_col.y, clear_col.z, 1.0f);
    glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Clear the renderable primitives.
    g_Draw.clear();

    /*-----------------------------------*/
    /*             Update                */
    /*-----------------------------------*/

    // Update camera
    OrbitCamera::Input cam_in;
    // AM: ImGui appears to take like a second to "spin up", and meanwhile, these appear to be true, hanging the input loop.
    //     ImGui is responsive during this time, but for whatever reason these are set 'true', so the orbit cam isn't. Need to fix.
    {//if (!ImGui::GetIO().WantCaptureMouse && !gui::ImGui().WantCaptureKeyboard) {
        cam_in.mouse_del = vec2(_in.mouse.dx, _in.mouse.dy);
        cam_in.wheel_del = _in.mouse.wheel;
        cam_in.rotate_down = _in.mouse.down[MOUSE_BUTTON_2];
    }
    s_camera.update(cam_in);

    // Compute transforms
    pose3 world_from_camera = s_camera.getWorldFromCamera();
    pose3 world_from_model = pose3::I();
    pose3 camera_from_model = inverse(world_from_camera) * world_from_model;
    float aspect = float(app.res_x) / app.res_y;
    float fovy = PI / 4;	
    mat4 proj_from_view = mat4PerspectiveProjection(fovy * 0.5f, aspect, 0.01f);
    mat4 view_from_model = mat4::fromPose(camera_from_model);

    g_Draw.addBox(vec3(0.0f), vec3(1.0f), vec3(1.0f, 0.0f, 0.0f));
    g_Draw.addBoxL(vec3(0.0f), vec3(1.01f), vec3(1.0f, 0.0f, 0.0f));

    vec3 world_hit_pos;
    if (intersectShapes(vec2(_in.mouse.x, _in.mouse.y), proj_from_view, inverse(view_from_model), mat4::fromPose(world_from_model), &world_hit_pos)) {
        g_Draw.addBox(world_hit_pos, vec3(0.05f), vec3(0.0f, 0.0f, 1.0f));
    }

    /*-----------------------------------*/
    /*             Render                */
    /*-----------------------------------*/

    renderResize(res);
    renderCheckShadersForChanges();

    renderBegin();

    renderSurfaceVisibilityBuffer(proj_from_view, view_from_model);

    g_Draw.updateGPU();
    FlatUniforms uni;
    uni.proj_from_model = proj_from_view * view_from_model;
    renderMesh(&g_Draw.lines.drawable, MESH_SHADER_FLAT, &uni);
    renderMesh(&g_Draw.triangles.drawable, MESH_SHADER_FLAT, &uni);
    renderEnd();

    /*-----------------------------------*/
    /*         Post & Present            */
    /*-----------------------------------*/
    renderPresent();

    return g_In.isPressed(inp::Escape);
}

