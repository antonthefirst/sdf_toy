#pragma once
#include "core/maths.h"
#include "core/container.h"
#include "core/hashmap.h"
#include "qef_paniq.h"
#include "qef_threed.h"

// Evaluate the SDF at a particular position.
// sdf_out  - signed distance field value output (null if not needed)
// grad_out - gradient of the signed distance field output (null if not needed)
// userdata - opaque userdata if needed
typedef void(*dcSdfEvalCallback)(vec3 pos_field, float* sdf_out, vec3* grad_out, void* userdata);

struct DualContouringSettings {
	enum EdgePlacement {
		AVERAGE,    // Basically SurfaceNets,
		QEF_PANIQ,  // Use QEF from paniq's shadertoy
		QEF_THREED, // Use QEF from ThreeD library
	};

	EdgePlacement edge_placement = QEF_THREED;
	bool clamp_verts_to_cell = false;
};
struct DualContouringResult {
	struct Triangle {
		int a,b,c; // index into verts
	};

	struct Quad {
		int a,b,c,d; // index into verts
	};

	Bunch<vec3> verts;
	Bunch<vec3> norms;
	Bunch<int> vert_ids;
	Bunch<Triangle> tris;
	Bunch<Quad> quads;
	ivec3 cell_count;

	// Debug
	Bunch<vec3> edge_crossings;
	Bunch<vec3> edge_crossing_gradients;
	Bunch<vec3> cell_centers;
};

inline bool edgeIntersection(dcSdfEvalCallback func, vec3 a, vec3 b, float* t) {
	float f_a, f_b;
	func(a, &f_a, 0, 0);
	func(b, &f_b, 0, 0);
	const int steps = 4;
	if (f_a >= 0.0f && f_b < 0.0f) {
		float l = length(b-a);
		vec3 n = (b-a) / l;
		vec3 x = a;
		for (int i = 0; i < steps; ++i) {
			x = x + n * f_a;
			func(x, &f_a, 0, 0);
		}
		*t = dot(x-a, n) / l;

		return true;
	}
	else if (f_b >= 0.0f && f_a < 0.0f) {
		
		float l = length(a-b);
		vec3 n = (a-b) / l;
		vec3 x = b;
		for (int i = 0; i < steps; ++i) {
			x = x + n * f_b;
			func(x, &f_b, 0, 0);
		}
		*t = 1.0f - dot(x-b, n) / l;

		return true;
	}
	else {
		return false;
	}
}

ivec3 vert3dId(int i, ivec3 count) {
	ivec3 ret;
	ret.z =  i / (count.x * count.y);
	ret.y = (i % (count.x * count.y)) / count.x;
	ret.x =  i % count.x;
	return ret;
}
int vert1dId(ivec3 p, ivec3 count) {
	return p.z * (count.x * count.y) + p.y * count.x + p.x;
}
vec3 vertPos(ivec3 id, ivec3 count, vec3 bound_min, vec3 bound_max) {
	vec3  st_center = st_from_id(id);
	vec3  uv_center = uv_from_st(st_center, vec3(count));
	vec3 cell_center = lerp(bound_min, bound_max, uv_center);
	return cell_center;
}

// Extracts the surface of an implicit function via dual contouring algorithm.
// res           - result output
// func          - sdf function to extract surface from
// bound_min/max - min/max bounds of the function around which to extract the surface
// cell_ext      - the extent (diameter) of the grid cell
inline void dualContouring(DualContouringResult* res, dcSdfEvalCallback func, const DualContouringSettings& set, vec3 bound_min, vec3 bound_max, float cell_ext) {
	vec3 bound_ext = bound_max - bound_min;
	vec3 bound_halfext = bound_ext * 0.5f;
	vec3 bound_center = (bound_min + bound_max) * 0.5f;

	ivec3 count = max(ivec3(bound_ext / cell_ext + vec3(1.0f)), ivec3(1));
	vec3 sample_bound_ext = vec3(count) * cell_ext;
	vec3 sample_bound_center = bound_center;
	vec3 sample_bound_half_ext = sample_bound_ext * 0.5f;
	vec3 sample_bound_min = sample_bound_center - sample_bound_half_ext;
	vec3 sample_bound_max = sample_bound_center + sample_bound_half_ext;

	res->cell_count = count;

	Hashmap<int, int> vert_id_to_idx;

	vec3 edge_crossings[12];
	vec3 edge_crossing_normals[12];
	for (int x = 0; x < count.x; ++x) 
	for (int y = 0; y < count.y; ++y) 		
	for (int z = 0; z < count.z; ++z) 
	{
		// index of the cell in grid
		ivec3 id = ivec3(x,y,z);
		// center of cell position in grid
		vec3  st_center = st_from_id(id);
		// normalized uv position inside the grid
		vec3  uv_center = uv_from_st(st_center, vec3(count));
		// Center-of-cell position within the bounds
		vec3 cell_center = lerp(sample_bound_min, sample_bound_max, uv_center);

		vec3 st_corner = vec3(id);
		vec3 uv_corner = uv_from_st(st_corner, vec3(count));
		vec3 cell_corner = lerp(sample_bound_min, sample_bound_max, uv_corner);


		// Corners of the cell surrounding pos_field
		vec3 s = vec3(cell_ext * 0.5f);
		vec3 c[8] = {
			cell_center + vec3(+s.x, +s.y, +s.z),   // 0 - Top 4
			cell_center + vec3(+s.x, -s.y, +s.z),	// 1
			cell_center + vec3(-s.x, -s.y, +s.z),	// 2
			cell_center + vec3(-s.x, +s.y, +s.z),	// 3
			cell_center + vec3(-s.x, -s.y, -s.z),	// 4 - Bottom 4
			cell_center + vec3(-s.x, +s.y, -s.z),	// 5
			cell_center + vec3(+s.x, +s.y, -s.z),	// 6
			cell_center + vec3(+s.x, -s.y, -s.z),	// 7
		}; 

		// -s,-s,-s (4) -> +s,-s,-s (7)
		// -s,-s,-s (4) -> -s,+s,-s (5)
		// -s,-s,-s (4) -> -s,-s,+s (2)

		vec3 edges[12][2] = { 
			{c[0], c[1]},
			{c[1], c[2]},
			{c[2], c[3]},
			{c[3], c[0]},

			{c[4], c[5]}, // 4: y axis 
			{c[5], c[6]},
			{c[6], c[7]},
			{c[7], c[4]}, // 7: x axis (negated) 

			{c[0], c[6]},
			{c[1], c[7]},
			{c[2], c[4]}, // 10: z axis (negated)
			{c[3], c[5]},
		};

		// For each cell, look at the edges at the corner, one for each axis. Use those to decide if we will make a quad around each one.
		//vec3  e_t;
		//bvec3 e_cross = bvec3(false);
		//e_cross.x = edgeIntersection(func, cell_corner, cell_corner + vec3(cell_ext, 0.0f, 0.0f), &e_t.x);
		//e_cross.y = edgeIntersection(func, cell_corner, cell_corner + vec3(0.0f, cell_ext, 0.0f), &e_t.y);
		//e_cross.z = edgeIntersection(func, cell_corner, cell_corner + vec3(0.0f, 0.0f, cell_ext), &e_t.z);

		ivec3 e_cross = ivec3(-1); // index of edge if it was crossed, otherwise -1 if uncrossed
		int edge_crossing_count = 0;
		for (int i = 0; i < 12; ++i) {
			float t;
			bool cross = edgeIntersection(func, edges[i][0], edges[i][1], &t);
			if (i == 4  && cross) e_cross.y = edge_crossing_count;
			if (i == 7  && cross) e_cross.x = edge_crossing_count;
			if (i == 10 && cross) e_cross.z = edge_crossing_count;
			if (cross) {
				edge_crossings[edge_crossing_count] = lerp(edges[i][0], edges[i][1], vec3(t));
				func(edge_crossings[edge_crossing_count], 0, &edge_crossing_normals[edge_crossing_count], 0);
				edge_crossing_normals[edge_crossing_count] = normalize(edge_crossing_normals[edge_crossing_count]);
				edge_crossing_count++;
			}
		}

		// If the normal is against the axis, reverse quad winding
		if (e_cross.x != -1 && id.y > 0 && id.z > 0) {
			if (dot(edge_crossing_normals[e_cross.x], vec3(1.0f, 0.0f, 0.0f)) >= 0.0f) {
				DualContouringResult::Quad q = {
					vert1dId(id,                     count),
					vert1dId(id + ivec3( 0, -1,  0), count),
					vert1dId(id + ivec3( 0, -1, -1), count),
					vert1dId(id + ivec3( 0,  0, -1), count),
				};
				res->quads.push(q);
			} else {
				DualContouringResult::Quad q = {
					vert1dId(id,                     count),
					vert1dId(id + ivec3( 0,  0, -1), count),
					vert1dId(id + ivec3( 0, -1, -1), count),
					vert1dId(id + ivec3( 0, -1,  0), count),
				};
				res->quads.push(q);
			}
		}

		if (e_cross.y != -1 && id.x > 0 && id.z > 0) {
			if (dot(edge_crossing_normals[e_cross.y], vec3(0.0f, 1.0f, 0.0f)) >= 0.0f) {
				DualContouringResult::Quad q = {
					vert1dId(id,                     count),
					vert1dId(id + ivec3( 0,  0, -1), count),
					vert1dId(id + ivec3(-1,  0, -1), count),
					vert1dId(id + ivec3(-1,  0,  0), count),
				};
				res->quads.push(q);
			} else {
				DualContouringResult::Quad q = {
					vert1dId(id,                     count),
					vert1dId(id + ivec3(-1,  0,  0), count),
					vert1dId(id + ivec3(-1,  0, -1), count),
					vert1dId(id + ivec3( 0,  0, -1), count),
				};
				res->quads.push(q);
			}
		}

		if (e_cross.z != -1 && id.x > 0 && id.y > 0) {
			if (dot(edge_crossing_normals[e_cross.z], vec3(0.0f, 0.0f, 1.0f)) >= 0.0f) {
				DualContouringResult::Quad q = {
					vert1dId(id,                     count),
					vert1dId(id + ivec3(-1,  0,  0), count),
					vert1dId(id + ivec3(-1, -1,  0), count),
					vert1dId(id + ivec3( 0, -1,  0), count),
				};
				res->quads.push(q);
			} else {
				DualContouringResult::Quad q = {
					vert1dId(id,                     count),
					vert1dId(id + ivec3( 0, -1,  0), count),
					vert1dId(id + ivec3(-1, -1,  0), count),
					vert1dId(id + ivec3(-1,  0,  0), count),
				};
				res->quads.push(q);
			}
		}

		for (int i = 0; i < edge_crossing_count; ++i) {
			res->edge_crossings.push(edge_crossings[i]);
			res->edge_crossing_gradients.push(edge_crossing_normals[i]);
		}

		if (edge_crossing_count) {
			res->cell_centers.push(cell_center);

			vec3 vert = vec3(0.0f);
			if (set.edge_placement == DualContouringSettings::EdgePlacement::AVERAGE) {
				for (int i = 0; i < edge_crossing_count; ++i)
					vert += edge_crossings[i];
				vert /= float(edge_crossing_count);
			}
			if (set.edge_placement == DualContouringSettings::EdgePlacement::QEF_PANIQ) {
				mat3 ATA = mat3(0.0);
				vec3 ATb = vec3(0.0);
				vec4 pointaccum = vec4(0.0);
				vec3 corner;
				float error;
				for (int i = 0; i < edge_crossing_count; ++i) {
					qef_add(edge_crossing_normals[i], edge_crossings[i], ATA, ATb, pointaccum);
				}

				error = qef_solve(ATA, ATb, pointaccum, corner);
				vert = corner;
			}
			if (set.edge_placement == DualContouringSettings::EdgePlacement::QEF_THREED) {
				vec3 mass_point = vec3(0.0f);
				for (int i = 0; i < edge_crossing_count; ++i)
					mass_point += edge_crossings[i];
				mass_point /= float(edge_crossing_count);

				double matrix_A[12][3];
				double vector_B[12];
				int rows = 0;

				for (int i = 0; i < edge_crossing_count; ++i) {
					// px,py,pz  is the intersection point
					// nx,ny,nz  is the normal vector at that point
					double px = (edge_crossings[i] - mass_point).x;
					double py = (edge_crossings[i] - mass_point).y;
					double pz = (edge_crossings[i] - mass_point).z;
					double nx = edge_crossing_normals[i].x;
					double ny = edge_crossing_normals[i].y;
					double nz = edge_crossing_normals[i].z;

					matrix_A[rows][0] = nx;
					matrix_A[rows][1] = ny;
					matrix_A[rows][2] = nz;

					// compute dot product
					vector_B[rows] = (double)(px * nx + py * ny + pz * nz);

					++rows;
				}

				// solve Ax=B using singular value decomposition
				ThreeD::QEF::evaluate(matrix_A, vector_B, rows, &vert);
				vert += mass_point;
			}

			if (set.clamp_verts_to_cell) {
				vec3 cell_min = cell_center - cell_ext * 0.5f;
				vec3 cell_max = cell_center + cell_ext * 0.5f;
				vert = max(cell_min, min(cell_max, vert));
			}

			vert_id_to_idx.set(vert1dId(id, count), res->verts.count);
			res->verts.push(vert);
			vec3 grad;
			func(vert, 0, &grad, 0);
			res->norms.push(normalize(grad));
		}
	}

	// Remap
	for (int i = 0; i < res->quads.count; ++i) {
		res->quads[i].a = vert_id_to_idx.getOr(res->quads[i].a, 0);
		res->quads[i].b = vert_id_to_idx.getOr(res->quads[i].b, 0);
		res->quads[i].c = vert_id_to_idx.getOr(res->quads[i].c, 0);
		res->quads[i].d = vert_id_to_idx.getOr(res->quads[i].d, 0);
	}

	// Split quads into triangles
	for (int i = 0; i < res->quads.count; ++i) {
		DualContouringResult::Triangle& t1 = res->tris.push();
		DualContouringResult::Triangle& t2 = res->tris.push();
		// The split will always be between a and c.
		// If the quad wants the split between b and d, they can shuffle the indecies of the quad to make that happen.
		t1.a = res->quads[i].a;
		t1.b = res->quads[i].b;
		t1.c = res->quads[i].c;
		t2.a = res->quads[i].c;
		t2.b = res->quads[i].d;
		t2.c = res->quads[i].a;
	}
}