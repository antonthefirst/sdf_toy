
float sdEgg(in vec2 p, float a, in float b, in float h)
{
	p.x = abs(p.x);
	float r = (a - b);
	h += r;
	float l = (h * h - r * r) / (2. * r);
	return ((p.y <= 0.) ? length(p) - a :
		((p.y - h) * l > p.x * h) ? length(p - vec2(0., h)) - ((a + l) - length(vec2(h, l))) :
		length(p + vec2(l, 0.)) - (a + l));
}


float sdCircle(vec2 p, float r) {
	return length(p) - r;
}

float sdOval(vec2 p, vec2 r) {
#if 0
	float a = r.x / r.y;
	return length(vec2(p.x, p.y * a)) - r.x;
#else
	float t = atan(p.y, p.x);
	vec2 s = vec2(r.x * cos(t), r.y * sin(t));
	vec2 ns = normalize(s);
	return dot(p - s, ns);
#endif
}

float sdMoon(vec2 p, float d, float ra, float rb)
{
	p.y = abs(p.y);

	float a = (ra * ra - rb * rb + d * d) / (2.0 * d);
	float b = sqrt(max(ra * ra - a * a, 0.0));
	if (d * (p.x * b - p.y * a) > d * d * max(b - p.y, 0.0))
	{
		return length(p - vec2(a, b));
	}

	return max((length(p) - ra),
		-(length(p - vec2(d, 0)) - rb));
}

float sdVesica(vec2 p, float r, float d)
{
	p = abs(p);
	float b = sqrt(r*r-d*d);
	return ((p.y-b)*d>p.x*b) ? length(p-vec2(0.0,b))
		: length(p-vec2(-d,0.0))-r;
}

float sdBox(vec2 p, vec2 s) {
	vec2 d = abs(p) - s;
	return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

float sdWedge(vec2 p, vec2 s) {
#if 0
	vec2 e = vec2(s.x * 2.0, -s.y);
	float el = sqrt(s.x * s.x * 4.0 + s.y * s.y);
	vec2 epn = perp(e / el); // #DODGY div by zero when s == 0?
	vec2 q = vec2(p.x, abs(p.y));
	float de = dot(q - vec2(s.x, 0.0), epn);
	float db = -p.x - s.x;
	return max(de, db);
#else
	vec2 f = vec2(s.x, 0.0);
	vec2 b = vec2(-s.x, s.y);
	vec2 e = f - b;
	vec2 ep = perp(e);
	vec2 epn = normalize(ep);
	vec2 q = vec2(p.x, abs(p.y));
	float de = dot(q - f, epn);
	float db = -p.x - s.x;
	return max(de, db);
#endif
}

float sdTriangleIsosceles(in vec2 p, in vec2 q)
{
	p.xy = p.yx;
	p.y += q.y;
	q.y *= 2.0;
	p.x = abs(p.x);
	vec2 a = p - q * clamp(dot(p, q) / dot(q, q), 0.0, 1.0);
	vec2 b = p - q * vec2(clamp(p.x / q.x, 0.0, 1.0), 1.0);
	float s = -sign(q.y);
	vec2 d = min(vec2(dot(a, a), s * (p.x * q.y - p.y * q.x)),
		vec2(dot(b, b), s * (p.y - q.y)));
	return -sqrt(d.x) * sign(d.y);
}

float sdTriangle( in vec2 p, in vec2 p0, in vec2 p1, in vec2 p2 )
{
	vec2 e0 = p1-p0, e1 = p2-p1, e2 = p0-p2;
	vec2 v0 = p -p0, v1 = p -p1, v2 = p -p2;
	vec2 pq0 = v0 - e0*clamp( dot(v0,e0)/dot(e0,e0), 0.0, 1.0 );
	vec2 pq1 = v1 - e1*clamp( dot(v1,e1)/dot(e1,e1), 0.0, 1.0 );
	vec2 pq2 = v2 - e2*clamp( dot(v2,e2)/dot(e2,e2), 0.0, 1.0 );
	float s = sign( e0.x*e2.y - e0.y*e2.x );
	vec2 d = min(min(vec2(dot(pq0,pq0), s*(v0.x*e0.y-v0.y*e0.x)),
		vec2(dot(pq1,pq1), s*(v1.x*e1.y-v1.y*e1.x))),
		vec2(dot(pq2,pq2), s*(v2.x*e2.y-v2.y*e2.x)));
	return -sqrt(d.x)*sign(d.y);
}

// Unused?
// from: https://www.iquilezles.org/www/articles/distfunctions2d/distfunctions2d.htm
float dCapsule(vec2 p, float r1, float r2, float h)
{
	p.xy = p.yx;
	p.x = abs(p.x);

	float b = (r1 - r2) / h;
	float a = sqrt(1.0 - b * b);
	float k = dot(p, vec2(-b, a));

	if (k < 0.0) return length(p) - r1;
	if (k > a * h) return length(p - vec2(0.0, h)) - r2;

	return dot(p, vec2(a, b)) - r1;
}

// Unused?
float sdCap(vec2 p, float r1, float r2, float h)
{
	p.x = abs(p.x);
	float b = (r1 - r2) / h;
	float a = sqrt(1.0 - b * b);
	float k = dot(p, vec2(-b, a));
	if (k < 0.0) return length(p) - r1;
	if (k > a * h) return length(p - vec2(0.0, h)) - r2;
	return dot(p, vec2(a, b)) - r1;
}

// Used
vec2 sdCapsule( vec2 p, float r1, float r2, float h )
{
	p.xy = p.yx;
	p.x = abs(p.x);
	float b = (r1-r2)/h;
	float a = sqrt(1.0-b*b);
	float k = dot(p,vec2(-b,a));
	float ah = a*h;
	if( k < 0.0 ) return vec2(length(p) - r1, 0.0);
	if( k > ah ) return vec2(length(p-vec2(0.0,h)) - r2, 1.0);
	return vec2(dot(p, vec2(a,b) ) - r1, clamp(k / ah, 0.0, 1.0));
}

float sdCrescent(vec2 p, float r, float r_cut) {
	float d0 = length(p) - r;
	float d1 = length(p-vec2(r-r_cut*0.5,0.0)) - r_cut;
	return max(d0, -d1);
}

float sdEllipse(in vec2 p, in vec2 ab)
{
	p = abs(p); if (p.x > p.y) { p = p.yx; ab = ab.yx; }
	float l = ab.y * ab.y - ab.x * ab.x;
	float m = ab.x * p.x / l;      float m2 = m * m;
	float n = ab.y * p.y / l;      float n2 = n * n;
	float c = (m2 + n2 - 1.0) / 3.0; float c3 = c * c * c;
	float q = c3 + m2 * n2 * 2.0;
	float d = c3 + m2 * n2;
	float g = m + m * n2;
	float co;
	if (d < 0.0)
	{
		float h = acos(q / c3) / 3.0;
		float s = cos(h);
		float t = sin(h) * sqrt(3.0);
		float rx = sqrt(-c * (s + t + 2.0) + m2);
		float ry = sqrt(-c * (s - t + 2.0) + m2);
		co = (ry + sign(l) * rx + abs(g) / (rx * ry) - m) / 2.0;
	} else
	{
		float h = 2.0 * m * n * sqrt(d);
		float s = sign(q + h) * pow(abs(q + h), 1.0 / 3.0);
		float u = sign(q - h) * pow(abs(q - h), 1.0 / 3.0);
		float rx = -s - u - c * 4.0 + 2.0 * m2;
		float ry = (s - u) * sqrt(3.0);
		float rm = sqrt(rx * rx + ry * ry);
		co = (ry / sqrt(rm - rx) + 2.0 * g / rm - m) / 2.0;
	}
	vec2 r = ab * vec2(co, sqrt(1.0 - co * co));
	return length(r - p) * sign(p.y - r.y);
}

float sdHexagon( in vec2 p, in float r )
{
	const vec3 k = vec3(-0.866025404,0.5,0.577350269);
	p = abs(p);
	p -= 2.0*min(dot(k.xy,p),0.0)*k.xy;
	p -= vec2(clamp(p.x, -k.z*r, k.z*r), r);
	return length(p)*sign(p.y);
}

float sdArrowhead(in vec2 p, in vec2 he, in float c)
{
#if 0
	p.x = abs(p.x);
	// Have to offset by 1 pixel here, so that there is no black gap, when doing interior distance.
	p.x += 1.0f;
	return sdTriangle(p,
		              vec2(0.0f, -he.y + c),
		              vec2(0.0f, +he.y),
		              vec2(+he.x, -he.y));
#else
	return min(sdTriangle(p,
		                  vec2(0.0f, -he.y + c),
		                  vec2(0.0f, +he.y),
		                  vec2(+he.x, -he.y)),
		       sdTriangle(p,
		       	          vec2(0.0f, -he.y + c),
		       	          vec2(0.0f, +he.y),
		       	          vec2(-he.x, -he.y)));
#endif
}

/* Modified from iq's: https://www.shadertoy.com/view/3tdSDj
Returns signed distance to segment and also the side (left/right = neg/pos) */
vec2 sdSegmentWithSign( in vec2 p, in vec2 a, in vec2 b )
{
	vec2 pa = p - a;
	vec2 ba = b - a;
	float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
	return vec2(length( pa - ba*h ), det(pa, ba));
}

#define CIRCUMSCRIBE 

float sdNgon(in vec2 p, in float r, in float n) {
	// can precompute these
	float inv_n = 1.0 / n;

	// perform radial repeat
	vec2 rp = vec2(atan(p.y, p.x), length(p)); // into polar coords
	rp.x *= 1.0 / TAU;
	rp.x = mod(rp.x + inv_n * 0.5, inv_n) - 0.5 * inv_n;
	rp.x *= TAU;
	p = vec2(cos(rp.x), sin(rp.x))*rp.y; // back to cartesian

#ifdef CIRCUMSCRIBE
	float s = cos(TAU * inv_n * 0.5); // scale by 1.0 / vertex_radius
	p /= s;
#endif
	// distance to a "box side"
	vec2 b = vec2(r);
	b.y = b.x * tan(TAU * inv_n * 0.5);
	vec2 d = abs(p)-b;

	float sd = length(max(d,vec2(0))) + min(d.x,0.0);

#ifdef CIRCUMSCRIBE
	return sd * s;
#else
	return sd;
#endif
}

float smin(float a, float b, float k)
{
	float res = exp2(-k * a) + exp2(-k * b);
	return -log2(res) / k;
}
vec3 smax( vec3 a, vec3 b, float k) {
	vec3 res = exp2(k*a) + exp2(k*b);
	return log2(res)/k;
}