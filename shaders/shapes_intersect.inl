float sphereTraceShapesSDF(vec3 ray_ori_field, vec3 ray_dir_field, float t_min, float t_max, int max_iter) {
	float t = t_min;
	for (int i = 0; i < max_iter; ++i) {
		vec3 pos_field = ray_ori_field + ray_dir_field*t;
		float f = ShapesSDF(pos_field);
		if (f < 0.001f) return t;
		if (t > t_max) return -1.0f;
		t += f * 0.5f;
	}
	return -1.0;
}