#ifdef _WIN32
//typedef unsigned int uint;
#endif

struct FlatUniforms {
	mat4 proj_from_model;
	float depth_offset;

#ifdef _WIN32
	FlatUniforms() {
		memset(this, 0, sizeof(FlatUniforms));
	}
#endif
};

// CPU
#ifdef _WIN32

// GPU
#else

uniform FlatUniforms uni;

#endif