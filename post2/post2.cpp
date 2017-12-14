#include "shared.h"

Float3 pathtrace_sample(ThreadContext &thread_context, const Scene &scene, const Camera &camera,
	uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t sample_index,
	float one_over_width, float one_over_height)
{
	const uint32_t image_index = thread_context.image_index;

	const float camera_x = (x + uniform(thread_context)) * one_over_width;
	const float camera_y = (y + uniform(thread_context)) * one_over_height;
	const Float3 camera_direction = generate_camera_direction(camera, camera_x, camera_y);

	Float3 pos = camera.position;
	Float3 dir = camera_direction;

	Float3 accumulated_color = float3(0,0,0);
	Float3 accumulated_importance = float3(1,1,1);

	for (uint32_t bounces = 0; bounces<100; bounces++) {
		IntersectResult intersect;
		if (!intersect_closest(scene, pos, dir, intersect)) {
			accumulated_color += accumulated_importance * sky_color_in_direction(scene, dir);
			break;
		}

		if (image_index == 0)
			return intersect.diffuse;

		accumulated_color += intersect.emissive * accumulated_importance;
		dir = random_hemisphere(intersect.face_normal, uniform(thread_context), uniform(thread_context));
		
		float area_hemisphere = float(2.0*M_PI);
		float probability_choosing_dir = 1.0f/area_hemisphere;
		float brdf_without_color = dot(dir, intersect.face_normal) * float(1.0/M_PI);

		accumulated_importance *= intersect.diffuse * (brdf_without_color / probability_choosing_dir);

		pos = intersect.pos + intersect.face_normal * 1E-6f; // Bias outward to avoid self-intersection
	}

	return accumulated_color;
}
