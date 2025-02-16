#ifdef _WIN32
typedef unsigned int uint;
#endif

struct ShapeVisibilityUniforms {
	mat4 proj_from_view;
	mat4 proj_from_model;
	mat4 view_from_proj;
	mat4 view_from_model;
	mat4 model_from_view;

#ifdef _WIN32
	ShapeVisibilityUniforms() {
		memset(this, 0, sizeof(ShapeVisibilityUniforms));
	}
#endif
};

// CPU
#ifdef _WIN32

// GPU
#else

uniform ShapeVisibilityUniforms uni;

#endif