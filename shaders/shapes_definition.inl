
// Some placeholder functions to mimic the real SDF for now.
float sdf3Box(vec3 p, vec3 b)
{
    vec3 q = abs(p) - b;
    return length(max(q, 0.0f)) + min(max(q.x, max(q.y, q.z)), 0.0f);
}
float sdf3Sphere(vec3 p, float r) {
    return length(p) - r;
}

// Returns the model space SDF
float ShapesSDF(vec3 p) {
    // #TODO read from actual buffer data.
    float sdf = 10000.0f;
    sdf = min(sdf, sdf3Box(p - vec3(0.0), vec3(1.0)));
    sdf = min(sdf, sdf3Box(p - vec3(1, 0, 0), vec3(0.5)));
    sdf = min(sdf, sdf3Box(p - vec3(0, 1, 0), vec3(0.5)));
    sdf = min(sdf, sdf3Box(p - vec3(0, 0, 1), vec3(0.5)));
    return sdf;
}

vec3 ShapesNormal(vec3 p) {
	const float h = 0.001f;
	const vec2 k = vec2(1,-1);
	vec3 g = vec3( 1.0f, -1.0f, -1.0f) * ShapesSDF(p + vec3( 1.0f, -1.0f, -1.0f) * h) + 
		     vec3(-1.0f, -1.0f,  1.0f) * ShapesSDF(p + vec3(-1.0f, -1.0f,  1.0f) * h) + 
		     vec3(-1.0f,  1.0f, -1.0f) * ShapesSDF(p + vec3(-1.0f,  1.0f, -1.0f) * h) + 
		     vec3( 1.0f,  1.0f,  1.0f) * ShapesSDF(p + vec3( 1.0f,  1.0f,  1.0f) * h);
	return normalize(g / (h*4.0));
}