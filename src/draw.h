#pragma once
#include "core/mesh.h"
#include "core/mesh_maker.h"
#include "render.h"

struct GeneralDrawer {
	SharedMesh triangles;
	SharedMesh lines;
	MeshMaker  triangles_mm = MeshMaker(&triangles.mesh);
	MeshMaker  lines_mm = MeshMaker(&lines.mesh);

	GeneralDrawer() {
		triangles.mesh.type = Mesh::Type::TRIANGLES;
		lines.mesh.type = Mesh::Type::LINES;
	}

	// Clears all the data.
	void clear();

	// Update the GPU buffers with the latest CPU data.
	// Must be called before rendering (otherwise it will draw stale).
	void updateGPU();

	void addBox(vec3 pos, vec3 s, vec3 col);
	void addBoxL(vec3 pos, vec3 s, vec3 col);

private:
	void addVert(MeshMaker& mm, vec3 a, vec3 n, vec3 col);
	void addTri(MeshMaker& mm, vec3 a, vec3 b, vec3 c, vec3 col, vec2 uv = vec2(0.0f));
};

inline void GeneralDrawer::addVert(MeshMaker& mm, vec3 a, vec3 n, vec3 col) {
	mm.vert(a.x, a.y, a.z, n.x, n.y, n.z, col.x, col.y, col.z, 1, 0, 0);
}
inline void GeneralDrawer::addTri(MeshMaker& mm, vec3 a, vec3 b, vec3 c, vec3 col, vec2 uv) {
	vec3 n = normalize(cross(b - a, c - a));
	mm.vert(a.x, a.y, a.z, n.x, n.y, n.z, col.x, col.y, col.z, 1, uv.x, uv.y);
	mm.vert(b.x, b.y, b.z, n.x, n.y, n.z, col.x, col.y, col.z, 1, uv.x, uv.y);
	mm.vert(c.x, c.y, c.z, n.x, n.y, n.z, col.x, col.y, col.z, 1, uv.x, uv.y);
}
inline void GeneralDrawer::addBox(vec3 pos, vec3 s, vec3 col) {
	MeshMaker& mm = triangles_mm;
	mm.shape();
	//back
	addTri(mm, vec3(pos.x + s.x, pos.y - s.y, pos.z - s.z), vec3(pos.x - s.x, pos.y - s.y, pos.z - s.z), vec3(pos.x - s.x, pos.y + s.y, pos.z - s.z), col);
	addTri(mm, vec3(pos.x + s.x, pos.y + s.y, pos.z - s.z), vec3(pos.x + s.x, pos.y - s.y, pos.z - s.z), vec3(pos.x - s.x, pos.y + s.y, pos.z - s.z), col);
	//front																																			 	
	addTri(mm, vec3(pos.x - s.x, pos.y - s.y, pos.z + s.z), vec3(pos.x + s.x, pos.y - s.y, pos.z + s.z), vec3(pos.x - s.x, pos.y + s.y, pos.z + s.z), col);
	addTri(mm, vec3(pos.x + s.x, pos.y - s.y, pos.z + s.z), vec3(pos.x + s.x, pos.y + s.y, pos.z + s.z), vec3(pos.x - s.x, pos.y + s.y, pos.z + s.z), col);
	//left																																			 	
	addTri(mm, vec3(pos.x - s.x, pos.y - s.y, pos.z + s.z), vec3(pos.x - s.x, pos.y + s.y, pos.z + s.z), vec3(pos.x - s.x, pos.y + s.y, pos.z - s.z), col);
	addTri(mm, vec3(pos.x - s.x, pos.y - s.y, pos.z + s.z), vec3(pos.x - s.x, pos.y + s.y, pos.z - s.z), vec3(pos.x - s.x, pos.y - s.y, pos.z - s.z), col);
	//right																																			 	
	addTri(mm, vec3(pos.x + s.x, pos.y + s.y, pos.z + s.z), vec3(pos.x + s.x, pos.y - s.y, pos.z + s.z), vec3(pos.x + s.x, pos.y + s.y, pos.z - s.z), col);
	addTri(mm, vec3(pos.x + s.x, pos.y + s.y, pos.z - s.z), vec3(pos.x + s.x, pos.y - s.y, pos.z + s.z), vec3(pos.x + s.x, pos.y - s.y, pos.z - s.z), col);
	//top																																			 	
	addTri(mm, vec3(pos.x - s.x, pos.y + s.y, pos.z + s.z), vec3(pos.x + s.x, pos.y + s.y, pos.z + s.z), vec3(pos.x - s.x, pos.y + s.y, pos.z - s.z), col);
	addTri(mm, vec3(pos.x + s.x, pos.y + s.y, pos.z + s.z), vec3(pos.x + s.x, pos.y + s.y, pos.z - s.z), vec3(pos.x - s.x, pos.y + s.y, pos.z - s.z), col);
	//bottom																																		  
	addTri(mm, vec3(pos.x + s.x, pos.y - s.y, pos.z + s.z), vec3(pos.x - s.x, pos.y - s.y, pos.z + s.z), vec3(pos.x - s.x, pos.y - s.y, pos.z - s.z), col);
	addTri(mm, vec3(pos.x + s.x, pos.y - s.y, pos.z - s.z), vec3(pos.x + s.x, pos.y - s.y, pos.z + s.z), vec3(pos.x - s.x, pos.y - s.y, pos.z - s.z), col);
	for (int i = 0; i < 12 * 3; i += 3)
		mm.elem(i, i + 1, i + 2);
}
inline void GeneralDrawer::addBoxL(vec3 pos, vec3 s, vec3 col) {
	MeshMaker& mm = lines_mm;
	mm.shape();
	vec3 n = vec3(0,0,1);
	addVert(mm, vec3(pos.x + s.x, pos.y + s.y, pos.z + s.z), n, col);
	addVert(mm, vec3(pos.x + s.x, pos.y + s.y, pos.z - s.z), n, col);
	addVert(mm, vec3(pos.x + s.x, pos.y - s.y, pos.z + s.z), n, col);
	addVert(mm, vec3(pos.x + s.x, pos.y - s.y, pos.z - s.z), n, col);

	addVert(mm, vec3(pos.x - s.x, pos.y + s.y, pos.z + s.z), n, col);
	addVert(mm, vec3(pos.x - s.x, pos.y + s.y, pos.z - s.z), n, col);
	addVert(mm, vec3(pos.x - s.x, pos.y - s.y, pos.z + s.z), n, col);
	addVert(mm, vec3(pos.x - s.x, pos.y - s.y, pos.z - s.z), n, col);

	mm.elem(0,1);
	mm.elem(1,3);
	mm.elem(3,2);
	mm.elem(2,0);

	mm.elem(4,5);
	mm.elem(5,7);
	mm.elem(7,6);
	mm.elem(6,4);

	mm.elem(4,0);
	mm.elem(5,1);
	mm.elem(6,2);
	mm.elem(7,3);
}
