#pragma once

#define _USE_MATH_DEFINES // TODO: Move to cmake

#include <stdint.h>
#include <cmath>
#include <cassert>
#include <algorithm>

struct Float2 { float x,y; };
struct Float3 { float x,y,z; };
struct Float4 { float x,y,z,w; };

inline Float3 normalized(Float3 d) {
	float len = sqrtf(d.x*d.x + d.y*d.y + d.z*d.z);
	float inv_len = 1.0f/len;
	d.x *= inv_len;
	d.y *= inv_len;
	d.z *= inv_len;
	return d;
}

inline Float2 float2(float x, float y) { Float2 r; r.x = x; r.y = y; return r; }
inline Float3 float3(float x, float y, float z) { Float3 r; r.x = x; r.y = y; r.z = z; return r; }
inline Float4 float4(float x, float y, float z, float w) { Float4 r; r.x = x; r.y = y; r.z = z; r.w = w; return r; }

inline void operator+=(Float3 &a, const Float3 o) { a.x+=o.x; a.y+=o.y; a.z+=o.z; }
inline void operator-=(Float3 &a, const Float3 o) { a.x-=o.x; a.y-=o.y; a.z-=o.z; }
inline void operator*=(Float3 &a, const Float3 o) { a.x*=o.x; a.y*=o.y; a.z*=o.z; }
inline void operator/=(Float3 &a, const float o) { a.x/=o; a.y/=o; a.z/=o; }
inline Float3 operator*(const Float3 a, const float m) { return float3(a.x*m, a.y*m, a.z*m); }
inline Float3 operator/(const Float3 a, const float m) { return float3(a.x/m, a.y/m, a.z/m); }
inline Float3 operator*(const float a, const Float3 b) { return float3(a*b.x, a*b.y, a*b.z); }
inline Float3 operator*(const Float3 a, const Float3 b) { return float3(a.x*b.x, a.y*b.y, a.z*b.z); }
inline Float3 operator+(const Float3 a, const Float3 b) { return float3(a.x+b.x, a.y+b.y, a.z+b.z); }
inline Float3 operator-(const Float3 a, const Float3 b) { return float3(a.x-b.x, a.y-b.y, a.z-b.z); }
inline Float3 operator-(const Float3 a) { return float3(-a.x, -a.y, -a.z); }
inline float mean(const Float3 a) { return (a.x+a.y+a.z)*(1.0f/3.0f); }
inline float max(const Float3 a) { return std::max(std::max(a.x, a.y), a.z); }
inline float clamp(float v, float m0, float m1) {
	if (v<m0) return m0;
	if (v>m1) return m1;
	return v;
}

inline Float3 saturate(const Float3 a) {
	Float3 r = a;
	if (r.x<0.0) r.x = 0.0;
	if (r.y<0.0) r.y = 0.0;
	if (r.z<0.0) r.z = 0.0;
	if (r.x>1.0) r.x = 1.0;
	if (r.y>1.0) r.y = 1.0;
	if (r.z>1.0) r.z = 1.0;
	return r;
}

inline float dot(Float3 a, Float3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }

inline Float3 cross(Float3 a, Float3 v) {
	float x = a.x, y = a.y, z = a.z;
	return float3(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x);
}

inline Float3 frame(Float3 normal, float x, float y, float z) {
	Float3 minor_axis = float3(0,0,0);
	if (fabs(normal.x) < fabs(normal.y)) {
		if (fabs(normal.x) < fabs(normal.z)) {
			minor_axis.x = 1.0f;
		} else {
			minor_axis.z = 1.0f;
		}
	} else {
		if (fabs(normal.y) < fabs(normal.z)) {
			minor_axis.y = 1.0f;
		} else {
			minor_axis.z = 1.0f;
		}
	}

	Float3 xaxis = normalized(cross(normal, minor_axis));
	const Float3 yaxis = cross(xaxis, normal);
	return xaxis*x + yaxis*y + normal*z;
}

inline Float3 random_cosine_hemisphere(Float3 normal, float u1, float u2) { // TODO: Rename u0, u1
	const float cos_theta = sqrtf(u2);
	const float sin_theta = sqrtf(std::max(1.0f-u2, 0.0f)); // TODO: Can underflow

	const float phi = float(2.0*M_PI) * u1;

	const float x = sin_theta * cosf(phi);
	const float y = sin_theta * sinf(phi);
	const float z = cos_theta;

	return frame(normal, x, y, z);
}

inline Float3 random_sphere(float u1, float u2) { // TODO: Rename u0, u1
	const float cos_theta = u2*2-1;
	const float sin_theta = sqrtf(1.0f-cos_theta*cos_theta); // TODO: Might underflow

	const float phi = float(2.0*M_PI)*u1;

	const float x = sin_theta*cosf(phi);
	const float y = sin_theta*sinf(phi);
	const float z = cos_theta;

	return float3(x, y, z);
}

inline Float3 random_hemisphere(Float3 normal, float u1, float u2) { // TODO: Rename u0, u1
	// First generate a direction on the whole sphere
	const Float3 sphere = random_sphere(u1, u2);
	return dot(sphere, normal) >= 0.0f ? sphere : -sphere;
}
