#include "render.h"
#include "math/maths.h"
#include "math/pose3.h"
#include "math/mat4.h"
#include "core/texture.h"
#include "core/platform.h"
#include "core/shader_loader.h"

// Shared struct definition for SDF params.
#include "shaders/visibility_shared.inl"

// The current resolution for which the renderer is targeted.
static ivec2 s_res = ivec2(0);

// These should be owned by studio.cpp (or any other app)
static unsigned int shape_pos_tex; // Position buffer. More wasteful than depth, but avoids discussion of normal vs reverse-z, for now
static unsigned int shape_nor_tex; // Normals buffer. Can be avoided by computing normals from depth as well.
static unsigned int shape_dep_tex; // Depth buffer. Redundant with positions but probably this is the one to keep.
static unsigned int shape_col_tex; // Color buffer. A stand-in for materials for now.
static unsigned int shape_fb;      // Framebuffer for fragment shader to render to. Not necessary if we use compute.

// These are purely for display purposes.
static unsigned int final_tex;

static unsigned int mesh_shaders[MESH_SHADER_COUNT];
static          int mesh_shader_generations[MESH_SHADER_COUNT];
static unsigned int viewport_shaders[VIEWPORT_SHADER_COUNT];
static unsigned int show_texture_shader;

typedef void(*SetUniformCallback)(int prog, void* userdata);

void SetShapeVisibilityUniforms(int prog, void* userdata) {
	ShapeVisibilityUniforms& uni = *(ShapeVisibilityUniforms*)userdata;
	glUniformMatrix4fv(glGetUniformLocation(prog, "uni.proj_from_view"), 1, GL_FALSE, (float*)&uni.proj_from_view);
	glUniformMatrix4fv(glGetUniformLocation(prog, "uni.view_from_proj"), 1, GL_FALSE, (float*)&uni.view_from_proj);
	glUniformMatrix4fv(glGetUniformLocation(prog, "uni.proj_from_model"), 1, GL_FALSE, (float*)&uni.proj_from_model);
	glUniformMatrix4fv(glGetUniformLocation(prog, "uni.view_from_model"), 1, GL_FALSE, (float*)&uni.view_from_model);
	glUniformMatrix4fv(glGetUniformLocation(prog, "uni.model_from_view"), 1, GL_FALSE, (float*)&uni.model_from_view);
}


void SetFlatUniforms(int prog, void* userdata) {
	FlatUniforms& uni = *(FlatUniforms*)userdata;
	int loc_proj_from_model = glGetUniformLocation(prog, "uni.proj_from_model");
	//int loc_view_from_model = glGetUniformLocation(prog, "view_from_model");

	//mat4 view_from_model_m = mat4::fromPose(view_from_model);
	//mat4 proj_from_model_m = proj_from_view * view_from_model_m;
	glUniformMatrix4fv(loc_proj_from_model, 1, GL_FALSE, (float*)&uni.proj_from_model);
	//glUniformMatrix4fv(loc_view_from_model, 1, GL_FALSE, (float*)&view_from_model_m);
}

static SetUniformCallback mesh_shader_uniform_setters[MESH_SHADER_COUNT] = {
	SetFlatUniforms,
	SetShapeVisibilityUniforms,
};

void SharedMesh::uploadCPUtoGPU() {
	if (!drawable.vbo)
		glGenBuffers(1, &drawable.vbo);
	if (!drawable.veo)
		glGenBuffers(1, &drawable.veo);

	if (mesh.getElemCount()) {
		// Push data to vbo/veo.
		glBindBuffer(GL_ARRAY_BUFFER, drawable.vbo);
		glBufferData(GL_ARRAY_BUFFER, mesh.getVertByteSize(), (void*)mesh.getVertPtr(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawable.veo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.getElemByteSize(), (void*)mesh.getElemPtr(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	} else {
		glBindBuffer(GL_ARRAY_BUFFER, drawable.vbo);
		glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawable.veo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	drawable.elem_count = mesh.getElemCount();
	switch (mesh.type) {
	case Mesh::TRIANGLES: drawable.prim = GL_TRIANGLES; break;
	case Mesh::LINES:     drawable.prim = GL_LINES;     break;
	case Mesh::POINTS:    drawable.prim = GL_POINTS;    break;
	};
}



void genShapeBuffers(ivec2 res) {
	// Color
	glGenTextures(1, &shape_col_tex);
	glBindTexture(GL_TEXTURE_2D, shape_col_tex);
	setTexStorageParams(TexDesc(res).setFormat(GL_RGB32F));
	glBindTexture(GL_TEXTURE_2D, 0);

	// Positions
	glGenTextures(1, &shape_pos_tex);
	glBindTexture(GL_TEXTURE_2D, shape_pos_tex);
	setTexStorageParams(TexDesc(res).setFormat(GL_RGB32F));
	glBindTexture(GL_TEXTURE_2D, 0);

	// Normals
	glGenTextures(1, &shape_nor_tex);
	glBindTexture(GL_TEXTURE_2D, shape_nor_tex);
	setTexStorageParams(TexDesc(res).setFormat(GL_RGB32F));
	glBindTexture(GL_TEXTURE_2D, 0);

	// Depth
	glGenTextures(1, &shape_dep_tex);
	glBindTexture(GL_TEXTURE_2D, shape_dep_tex);
	setTexStorageParams(TexDesc(res).setFormat(GL_DEPTH_COMPONENT32F));
	glBindTexture(GL_TEXTURE_2D, 0);

	// Framebuffer
	glGenFramebuffers(1, &shape_fb);
	glBindFramebuffer(GL_FRAMEBUFFER, shape_fb);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shape_col_tex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, shape_nor_tex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, shape_pos_tex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, shape_dep_tex, 0);
	ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void deleteShapeBuffers() {
	glDeleteTextures(1, &shape_col_tex);
	shape_col_tex = 0;

	glDeleteTextures(1, &shape_pos_tex);
	shape_pos_tex = 0;

	glDeleteTextures(1, &shape_nor_tex);
	shape_nor_tex = 0;

	glDeleteTextures(1, &shape_dep_tex);
	shape_dep_tex = 0;

	glDeleteFramebuffers(1, &shape_fb);
	shape_fb = 0;
}


void showTexture(unsigned int tex) {
	if (show_texture_shader) {
		glUseProgram(show_texture_shader);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);

		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, tex);
		glUniform1i(glGetUniformLocation(show_texture_shader, "tex"), 0);

		// For the "one triangle" version
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// For the 4-vert triangle strip version. Doesn't work, for some reason.
		//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDepthFunc(GL_LESS);
		glDisable(GL_DEPTH_TEST);
	}
}

void drawWithViewportShader(int viewport_shader_id, SetUniformCallback setUniforms, void* uniform_data) {
	int prog = viewport_shaders[viewport_shader_id];
	if (prog) {
		glUseProgram(prog);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);

		setUniforms(prog, uniform_data);

		// For the "one triangle" version
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// For the 4-vert triangle strip version. Doesn't work, for some reason.
		//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDepthFunc(GL_LESS);
		glDisable(GL_DEPTH_TEST);
	}
}


void renderResize(ivec2 res) {
	// Don't do anything if asked to resize to zero, because this happens on window minimization
	if (all(res == ivec2(0)))
		return;

	if (all(res == s_res))
		return;

	// Destroy existing buffers
	deleteShapeBuffers();

	// Create new buffers for the resolution
	genShapeBuffers(res);

	s_res = res;
}

//unsigned int VisibilityShader::prog = 0;

void renderCheckShadersForChanges() {
	ProgramStats stats;

	show_texture_shader = getProgram("shaders/show_texture.vert", "shaders/show_texture.frag", &stats);
	if (stats.rebuild_occurred) {
		// nothing to do on change.
	}

	viewport_shaders[VIEWPORT_SHADER_SHAPE_VISIBILITY] = getProgram("shaders/visibility.vert", "shaders/visibility.frag", &stats);
	if (stats.rebuild_occurred) {
		// nothing to do on change.
	}

	mesh_shaders[MESH_SHADER_FLAT] = getProgram("shaders/flat.vert", "shaders/flat.frag", &stats);
	if (stats.rebuild_occurred) {
		mesh_shader_generations[MESH_SHADER_FLAT] += 1;
	}
}

void renderBegin() {
	glBindFramebuffer(GL_FRAMEBUFFER, shape_fb);

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, buffers);

	glViewport(0, 0, s_res.x, s_res.y);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderEnd() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderPresent() {
	showTexture(shape_col_tex);
}

void renderSurfaceVisibilityBuffer(mat4 proj_from_view, mat4 view_from_model) {

	ShapeVisibilityUniforms uni;
	uni.proj_from_view = proj_from_view;
	uni.view_from_proj = inverse(proj_from_view);
	uni.proj_from_model = proj_from_view * view_from_model;
	uni.view_from_model = view_from_model;
	uni.model_from_view = inverse(view_from_model);
	drawWithViewportShader(VIEWPORT_SHADER_SHAPE_VISIBILITY, SetShapeVisibilityUniforms, &uni);
}

void setAttribPointer(int prog, const char* name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer) {
	int loc = glGetAttribLocation(prog, name);
	if (loc >= 0) {
		glVertexAttribPointer    (loc, size, type, normalized, stride, pointer);
		glEnableVertexAttribArray(loc);
	} else {
		print("Attribute '%s' is inactive!", name);
	}
}
void setAttribIPointer(int prog, const char* name, GLint size, GLenum type, GLsizei stride, const void * pointer) {
	int loc = glGetAttribLocation(prog, name);
	if (loc >= 0) {
		glVertexAttribIPointer    (loc, size, type, stride, pointer);
		glEnableVertexAttribArray(loc);
	} else {
		print("Attribute '%s' is inactive!", name);
	}
}

static void remakeVAOAndBind(unsigned int* vao, unsigned int vbo, unsigned int veo) {
	// Can probably recycle it, but I'm worried what might happen during hot reloading on C++ side (straggler attrib's bound?)
	glDeleteVertexArrays(1, vao);
	glGenVertexArrays(1, vao);

	glBindVertexArray(*vao);

	glBindBuffer(GL_ARRAY_BUFFER,         vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veo);
}
void recreateMeshVAOForFlatShader(DrawableMesh* drawable, MeshShaderId shader_id) {
	int prog = mesh_shaders[shader_id];
	if (!prog) {
		ASSERT(false);
		return;
	}
	remakeVAOAndBind(&drawable->vaos[shader_id], drawable->vbo, drawable->veo);
	setAttribPointer(prog, "in_pos_local", 3, GL_FLOAT, GL_FALSE, (GLsizei)MeshVert::getStride(), (GLvoid*)MeshVert::getOffPos());
	setAttribPointer(prog, "in_nor_local", 3, GL_FLOAT, GL_FALSE, (GLsizei)MeshVert::getStride(), (GLvoid*)MeshVert::getOffNorm());
	setAttribPointer(prog, "in_col", 4, GL_FLOAT, GL_FALSE, (GLsizei)MeshVert::getStride(), (GLvoid*)MeshVert::getOffCol());
	setAttribPointer(prog, "in_uv", 2, GL_FLOAT, GL_FALSE, (GLsizei)MeshVert::getStride(), (GLvoid*)MeshVert::getOffUv());
	//setAttribIPointer(prog, "in_data", 1, GL_UNSIGNED_INT, (GLsizei)MeshVert::getStride(), (GLvoid*)MeshVert::getOffData());
	glBindVertexArray(0);
	drawable->vao_generation[shader_id] = mesh_shader_generations[shader_id];
}
void renderMesh(DrawableMesh* drawable, MeshShaderId shader_id, void* uniform_data) {
	if (drawable->vao_generation[shader_id] < mesh_shader_generations[shader_id]) {
		recreateMeshVAOForFlatShader(drawable, shader_id);
	}

	int prog = mesh_shaders[shader_id];
	if (prog == 0) {
		// Program isn't available.
		ASSERT(false);
		return;
	}
	if (drawable->elem_count == 0) {
		// Nothing to draw.
		return;
	}
	if (drawable->vaos[shader_id] == 0) {
		// Drawable doesn't support this shader.
		ASSERT(false);
		return;
	}

	// #TODO disable?
	if (drawable->prim == GL_TRIANGLES) {
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);
	}

	glUseProgram(prog);

	// + Alpha blend
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// + Depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GREATER);

	glBindVertexArray(drawable->vaos[shader_id]);

	mesh_shader_uniform_setters[shader_id](prog, uniform_data);

	// #TODO Indexing type maybe needs to be in MeshDrawable?
	glDrawElements(drawable->prim, drawable->elem_count, GL_UNSIGNED_INT, (void*)(0 * sizeof(GLuint))); 

	glBindVertexArray(0);

	// - Depth test
	glDepthFunc(GL_LESS);
	glDisable(GL_DEPTH_TEST);

	// - Alpha blend
	glDisable(GL_BLEND);
}
