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
inline Float3 operator*(const Float3 a, const float m) { return float3(a.x*m, a.y*m, a.z*m); }
inline Float3 operator*(const float a, const Float3 b) { return float3(a*b.x, a*b.y, a*b.z); }
inline Float3 operator*(const Float3 a, const Float3 b) { return float3(a.x*b.x, a.y*b.y, a.z*b.z); }
inline Float3 operator+(const Float3 a, const Float3 b) { return float3(a.x+b.x, a.y+b.y, a.z+b.z); }
inline Float3 operator-(const Float3 a, const Float3 b) { return float3(a.x-b.x, a.y-b.y, a.z-b.z); }
inline Float3 operator-(const Float3 a) { return float3(-a.x, -a.y, -a.z); }

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

// TODO: While it is ok to have a naive RNG at first we must at least have something that is threadable
//       A per-thread context makes sense but if we are going to deterministic numbers then we might not need it
inline float uniform() {
	return rand()/(float)RAND_MAX;
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
