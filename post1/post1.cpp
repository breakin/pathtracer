#include "shared.h"

Float3 pathtrace_sample(ThreadContext &thread_context, const Scene &scene, const Camera &camera,
	uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t sample_index,
	float one_over_width, float one_over_height)
{
	const float camera_x = (x + uniform(thread_context)) * one_over_width;
	const float camera_y = (y + uniform(thread_context)) * one_over_height;
	const Float3 camera_direction = generate_camera_direction(camera, camera_x, camera_y);

	const bool use_russian_roulette = thread_context.image_index != 2;
	const bool just_diffuse = thread_context.image_index == 0;
	
	Float3 pos = camera.position;
	Float3 dir = camera_direction;

	Float3 accumulated_color = float3(0,0,0);
	Float3 accumulated_importance = float3(1,1,1);

	for (uint32_t depth = 0;; depth++) {
		IntersectResult intersect;
		if (!intersect_closest(scene, pos, dir, intersect)) {
			accumulated_color += accumulated_importance * sky_color_in_direction(scene, dir);
			break;
		}

		if (just_diffuse)
			return intersect.diffuse;

		accumulated_color += intersect.emissive * accumulated_importance;
		accumulated_importance *= intersect.diffuse;

		if (use_russian_roulette) {
			float m = clamp(mean(accumulated_importance), 0.05f, 0.98f);
			float p = uniform(thread_context);
			if (p>m)
				break;
			accumulated_importance.x /= m;
			accumulated_importance.y /= m;
			accumulated_importance.z /= m;
		} else if (depth > 100) {
			break;
		}

		pos = intersect.pos + intersect.face_normal * 1E-6f; // Bias outward to avoid self-intersection
		dir = random_cosine_hemisphere(intersect.face_normal, uniform(thread_context), uniform(thread_context));
	}

	return accumulated_color;
}
