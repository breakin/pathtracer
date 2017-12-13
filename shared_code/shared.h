#pragma once

#include "vector_math.h"

#include <stdint.h>
#include <cassert>

/*
	Assumes elements are POD-structs. No constructor/destructor.
	Destructors will not be called and objects will be moved using memcpy.
*/
template<typename ELEMENT>
class Array {
public:
	Array(uint32_t initial_size = 0, bool clear_elements = false) : _size(0), _capacity(0), _elements(nullptr) {
		if (initial_size != 0)
			resize(initial_size, clear_elements);
	}
	~Array() {
		if (_elements) {
			delete [] _elements;
		}
	}
	ELEMENT &operator[](const uint32_t idx) { assert(_size != 0 && idx < _size); return _elements[idx]; }
	const ELEMENT &operator[](const uint32_t idx) const { assert(_size != 0 && idx < _size); return _elements[idx]; }
	void clear_elements() {
		if (_elements)
			memset(_elements, 0, _size*sizeof(ELEMENT));
	}
	void clear() {
		delete [] _elements;
		_elements = nullptr;
		_capacity = _size = 0;
	}
	uint32_t size() const { return _size; }
	void push_back(const ELEMENT &element) {
		if (_size == _capacity) {
			reserve(1+_size*2);
		}
		_elements[_size++] = element;
	}
	void resize(uint32_t new_size, bool clear_new_elements = false) {
		if (new_size <= _size) {
			_size = new_size;
			return;
		}
		reserve(new_size); // Make sure we have room
		assert(_capacity >= new_size);
		if (clear_new_elements) {
			assert(new_size > _size);
			memset(_elements + _size, 0, (new_size-_size)*sizeof(ELEMENT));
		}
	}
	void reserve(uint32_t new_capacity) {
		const uint32_t old_capacity = _capacity;
		uint32_t d = new_capacity - old_capacity;
		if (new_capacity <= old_capacity) {
			// We don't shrink in reserve
			return;
		}
		ELEMENT *ne = new ELEMENT[new_capacity];
		memcpy(ne, _elements, _size * sizeof(ELEMENT));
		delete[] _elements;
		_elements = ne;
		_capacity = new_capacity;
	}
private:
	uint32_t _size, _capacity;
	ELEMENT *_elements;
};

struct Pixel {
	Float3 rgb;
	uint32_t N;
};

// Opaque to the posts (for now)
struct Scene;

struct IntersectResult {
	Float3 diffuse, emissive;
	Float3 pos, face_normal/*, interpolated_normal*/;
};

// Until we do DOF we don't need anything more advanced than this
// Notice that this is a 4x3 matrix in disguise
// Also we don't do aspect ratio (yet)
struct Camera {
	Float3 position, forward, up, right;
};

inline Float3 generate_camera_direction(const Camera &camera, float u, float v) {
	Float3 d = camera.forward;
	d += camera.right * (u*2.0f-1.0f);
	d += camera.up * (v*2.0f-1.0f);
	return normalized(d);
}

inline Float3 sky_color_in_direction(const Scene &scene, const Float3 dir) { return float3(0.9f, 0.9f, 1.2f); }

bool intersect_closest(const Scene &scene, const Float3 pos, const Float3 dir, IntersectResult &out_result);

/*
	This is where we stick per-thread information such as seed for random number generators

*/
struct ThreadContext {
	uint32_t thread_index;
	uint32_t image_index = 0;
};

// TODO: Turn this into something proper
inline float uniform(ThreadContext &tc) {
	return rand()/(float)RAND_MAX;
}

// To be implemented by post
Float3 pathtrace_sample(ThreadContext &settings, const Scene &scene, const Camera &camera, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t sample_index, float one_over_width, float one_over_height);
