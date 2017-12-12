#include "shared.h"

#define MAX_DEPTH 4

// We need to be able to store data structures of say lights or so somewhere
// Post-specific data-structures..  So lets make init and a pointer somehow
Float3 pathtrace_pixel(const Scene &scene, const Camera &camera, uint32_t x, uint32_t y, uint32_t width, uint32_t height, float u0, float v0, float uw, float vw) {
	const Float3 camera_direction = generate_camera_direction(camera, u0, v0);
	
	Float3 pos = camera.position, dir = camera_direction;

	const Float3 sky_color = float3(0.7f, 0.7f, 1.2f); // TODO: Scene is currently opaque, but where do we put lights and this one?

	Float3 accumulated_color = float3(0,0,0);
	Float3 accumulated_importance = float3(1,1,1);

	for (uint32_t depth = 0; depth < MAX_DEPTH; depth++) {
		IntersectResult intersect;
		if (!intersect_closest(scene, pos, dir, intersect)) {
			accumulated_color += accumulated_importance * sky_color;
			break;
		}

		accumulated_color += intersect.emissive * accumulated_importance;
		accumulated_importance *= intersect.diffuse;

		pos = intersect.pos + intersect.face_normal * 1E-6f; // Bias outward to avoid self-intersection
		dir = random_cosine_hemisphere(intersect.face_normal, uniform(), uniform());

		// TODO: Russian roulette
	}

	return accumulated_color;
}
