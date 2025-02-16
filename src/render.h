#pragma once
#include "math/maths.h"
#include "math/mat4.h"
#include "core/mesh.h"

// Shared uniforms headers to setup rendering.
#include "shaders/flat_shared.inl"

// Meshes may be rendered with a variety of shaders, therefore a GPUMesh needs VAO's for them all.
enum MeshShaderId {
	MESH_SHADER_FLAT,
	MESH_SHADER_LIT,
	MESH_SHADER_COUNT,
};

struct DrawableMesh {
	// A VAO for each shader which this can draw with (entry is 0 if cannot be used).
	unsigned int vaos[MESH_SHADER_COUNT];
	// This is used to detect shader rebuilds. If a shader is rebuilt, it's internal generation is incremented, and if this doesn't match, VAO will be rebuilt.
	int vao_generation[MESH_SHADER_COUNT];
	// The VBO/VEO. Not strictly needed for rendering, but needed for automatically recreating the VAO in case of shader change.
	unsigned int vbo = 0;
	unsigned int veo = 0;
	// The primitive to draw with.
	unsigned int prim = 0; // POINTS
	// Number of elements to draw.
	unsigned int elem_count = 0;

	DrawableMesh() {
		for (int i = 0; i < MESH_SHADER_COUNT; ++i) {
			// Null handle.
			vaos[i] = 0;
			// Signal that VAO needs to be rebuilt on the first round.
			vao_generation[i] = -1;
		}
	}
};

// Mesh shared between CPU and GPU
struct SharedMesh {
	Mesh mesh;
	DrawableMesh drawable;

	void uploadCPUtoGPU();
};

// Shaders which run on the entire viewport.
enum ViewportShaderId {
	VIEWPORT_SHADER_SHAPE_VISIBILITY,
	VIEWPORT_SHADER_COUNT,
};

// Initialize all the necessary buffers and shaders for a given resolution
void renderResize(ivec2 res);

// Check if any shader has been updated
void renderCheckShadersForChanges();

// API TBD.
// Render the surface defined by the prim tree into some specified buffer, from some specified camera angle.
// This should be done using shaders from the engine, the app is just responsible in setting them up.
void renderSurfaceVisibilityBuffer(mat4 proj_from_view, mat4 view_from_model);

// Bind the render buffer
void renderBegin();

// Unbind the render buffer
void renderEnd();

// Draw the resulting buffer.
void renderPresent();


// Render a mesh.
void renderMesh(DrawableMesh* drawable, MeshShaderId shader_id, void* uniform_data);