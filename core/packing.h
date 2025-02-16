#pragma once

inline u32 pack_unorm(float x, float y, float z, float w) {
#define TOBYTE(x) (int(max(0.f, min(255.f, x*255.f + 0.5f))))
	return (TOBYTE(w) << 24) | (TOBYTE(z) << 16) | (TOBYTE(y) << 8) | (TOBYTE(x));
}
template<typename T>
inline u32 pack_unorm(T v) {
#define TOBYTE(x) (int(max(0.f, min(255.f, x*255.f + 0.5f))))
	return (TOBYTE(v.w) << 24) | (TOBYTE(v.z) << 16) | (TOBYTE(v.y) << 8) | (TOBYTE(v.x));
}
