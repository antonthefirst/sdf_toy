#ifdef _WIN32
//typedef unsigned int uint;
#endif

struct ShowTextureUniforms {
	uint pad;
#ifdef _WIN32
	ShowTextureUniforms() {
		memset(this, 0, sizeof(ShowTextureUniforms));
	}
#endif
};

// CPU
#ifdef _WIN32

// GPU
#else

uniform ShowTextureUniforms uni;

#endif