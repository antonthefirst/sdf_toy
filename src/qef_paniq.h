#pragma once
// from: https://www.shadertoy.com/view/4tsGRl
// minimal SVD implementation for calculating feature points from hermite data
// works in C++ and GLSL

// SVD/QEF parts are public domain

#define SVD_NUM_SWEEPS 10

// GLSL prerequisites

#ifdef _WIN32
#include "core/maths.h"
#include "core/mat3.h"
#define IN(t,x) t x
#define OUT(t, x) t& x
#define INOUT(t, x) t& x
float inline rsqrt(float x) {
    return 1.0f / sqrtf(x);
}
#define SWIZZLE_XYZ(v) v.xyz()
#define INLINE inline
#else
#define IN(t,x) in t x
#define OUT(t, x) out t x
#define INOUT(t, x) inout t x
#define rsqrt inversesqrt

#define SWIZZLE_XYZ(v) v.xyz
#define INLINE 
#endif

// SVD
////////////////////////////////////////////////////////////////////////////////


INLINE void givens_coeffs_sym(float a_pp, float a_pq, float a_qq, OUT(float,c), OUT(float,s)) {
    if (a_pq == 0.0) {
        c = 1.0;
        s = 0.0;
        return;
    }
    float tau = (a_qq - a_pp) / (2.0 * a_pq);
    float stt = sqrt(1.0 + tau * tau);
    float tan = 1.0 / ((tau >= 0.0) ? (tau + stt) : (tau - stt));
    c = rsqrt(1.0 + tan * tan);
    s = tan * c;
}

INLINE void svd_rotate_xy(INOUT(float,x), INOUT(float,y), IN(float,c), IN(float,s)) {
    float u = x; float v = y;
    x = c * u - s * v;
    y = s * u + c * v;
}

INLINE void svd_rotateq_xy(INOUT(float,x), INOUT(float,y), INOUT(float,a), IN(float,c), IN(float,s)) {
    float cc = c * c; float ss = s * s;
    float mx = 2.0 * c * s * a;
    float u = x; float v = y;
    x = cc * u - mx + ss * v;
    y = ss * u + mx + cc * v;
}

INLINE void svd_rotate01(INOUT(mat3,vtav), INOUT(mat3,v)) {
    if (vtav[0][1] == 0.0) return;

    float c, s;
    givens_coeffs_sym(vtav[0][0], vtav[0][1], vtav[1][1], c, s);
    svd_rotateq_xy(vtav[0][0],vtav[1][1],vtav[0][1],c,s);
    svd_rotate_xy(vtav[0][2], vtav[1][2], c, s);
    vtav[0][1] = 0.0;

    svd_rotate_xy(v[0][0], v[0][1], c, s);
    svd_rotate_xy(v[1][0], v[1][1], c, s);
    svd_rotate_xy(v[2][0], v[2][1], c, s);
}

INLINE void svd_rotate02(INOUT(mat3,vtav), INOUT(mat3,v)) {
    if (vtav[0][2] == 0.0) return;

    float c, s;
    givens_coeffs_sym(vtav[0][0], vtav[0][2], vtav[2][2], c, s);
    svd_rotateq_xy(vtav[0][0],vtav[2][2],vtav[0][2],c,s);
    svd_rotate_xy(vtav[0][1], vtav[1][2], c, s);
    vtav[0][2] = 0.0;

    svd_rotate_xy(v[0][0], v[0][2], c, s);
    svd_rotate_xy(v[1][0], v[1][2], c, s);
    svd_rotate_xy(v[2][0], v[2][2], c, s);
}

INLINE void svd_rotate12(INOUT(mat3,vtav), INOUT(mat3,v)) {
    if (vtav[1][2] == 0.0) return;

    float c, s;
    givens_coeffs_sym(vtav[1][1], vtav[1][2], vtav[2][2], c, s);
    svd_rotateq_xy(vtav[1][1],vtav[2][2],vtav[1][2],c,s);
    svd_rotate_xy(vtav[0][1], vtav[0][2], c, s);
    vtav[1][2] = 0.0;

    svd_rotate_xy(v[0][1], v[0][2], c, s);
    svd_rotate_xy(v[1][1], v[1][2], c, s);
    svd_rotate_xy(v[2][1], v[2][2], c, s);
}

INLINE void svd_solve_sym(IN(mat3,a), OUT(vec3,sigma), INOUT(mat3,v)) {
    // assuming that A is symmetric: can optimize all operations for 
    // the lower left triagonal
    mat3 vtav = a;
    // assuming V is identity: you can also pass a matrix the rotations
    // should be applied to
    // U is not computed
    for (int i = 0; i < SVD_NUM_SWEEPS; ++i) {
        svd_rotate01(vtav, v);
        svd_rotate02(vtav, v);
        svd_rotate12(vtav, v);
    }
    sigma = vec3(vtav[0][0],vtav[1][1],vtav[2][2]);    
}

INLINE float svd_invdet(float x, float tol) {
    return (abs(x) < tol || abs(1.0 / x) < tol) ? 0.0 : (1.0 / x);
}

INLINE void svd_pseudoinverse(OUT(mat3,o), IN(vec3,sigma), IN(mat3,v)) {
    const float Tiny_Number = 0.001f;
    float d0 = svd_invdet(sigma[0], Tiny_Number);
    float d1 = svd_invdet(sigma[1], Tiny_Number);
    float d2 = svd_invdet(sigma[2], Tiny_Number);
    o = mat3(v[0][0] * d0 * v[0][0] + v[0][1] * d1 * v[0][1] + v[0][2] * d2 * v[0][2],
             v[0][0] * d0 * v[1][0] + v[0][1] * d1 * v[1][1] + v[0][2] * d2 * v[1][2],
             v[0][0] * d0 * v[2][0] + v[0][1] * d1 * v[2][1] + v[0][2] * d2 * v[2][2],
             v[1][0] * d0 * v[0][0] + v[1][1] * d1 * v[0][1] + v[1][2] * d2 * v[0][2],
             v[1][0] * d0 * v[1][0] + v[1][1] * d1 * v[1][1] + v[1][2] * d2 * v[1][2],
             v[1][0] * d0 * v[2][0] + v[1][1] * d1 * v[2][1] + v[1][2] * d2 * v[2][2],
             v[2][0] * d0 * v[0][0] + v[2][1] * d1 * v[0][1] + v[2][2] * d2 * v[0][2],
             v[2][0] * d0 * v[1][0] + v[2][1] * d1 * v[1][1] + v[2][2] * d2 * v[1][2],
             v[2][0] * d0 * v[2][0] + v[2][1] * d1 * v[2][1] + v[2][2] * d2 * v[2][2]);
}

INLINE void svd_solve_ATA_ATb(
    IN(mat3,ATA), IN(vec3,ATb), OUT(vec3,x)
) {
    mat3 V = mat3(1.0);
    vec3 sigma;

    svd_solve_sym(ATA, sigma, V);

    mat3 Vinv;
    svd_pseudoinverse(Vinv, sigma, V);
    x = Vinv * ATb;
}

INLINE vec3 svd_vmul_sym(IN(mat3,a), IN(vec3,v)) {
    return vec3(
        dot(a[0],v),
        (a[0][1] * v.x) + (a[1][1] * v.y) + (a[1][2] * v.z),
        (a[0][2] * v.x) + (a[1][2] * v.y) + (a[2][2] * v.z)
    );
}

// QEF
////////////////////////////////////////////////////////////////////////////////

INLINE void qef_add(
    IN(vec3,n), IN(vec3,p),
    INOUT(mat3,ATA), 
    INOUT(vec3,ATb),
    INOUT(vec4,pointaccum))
{
    ATA[0][0] += n.x * n.x;
    ATA[0][1] += n.x * n.y;
    ATA[0][2] += n.x * n.z;
    ATA[1][1] += n.y * n.y;
    ATA[1][2] += n.y * n.z;
    ATA[2][2] += n.z * n.z;

    float b = dot(p, n);
    ATb += n * b;
    pointaccum += vec4(p,1.0);
}

INLINE float qef_calc_error(IN(mat3,A), IN(vec3, x), IN(vec3, b)) {
    vec3 vtmp = b - svd_vmul_sym(A, x);
    return dot(vtmp,vtmp);
}

INLINE float qef_solve(
    IN(mat3,ATA), 
    IN(vec3,ATb),
    IN(vec4,pointaccum),
    OUT(vec3,x)
) {
    vec3 masspoint = SWIZZLE_XYZ(pointaccum) / pointaccum.w;
    ATb -= svd_vmul_sym(ATA, masspoint);
    svd_solve_ATA_ATb(ATA, ATb, x);
    float result = qef_calc_error(ATA, x, ATb);

    x += masspoint;

    return result;
}
