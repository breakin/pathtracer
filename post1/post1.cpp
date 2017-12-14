#include "shared.h"

Float3 pathtrace_sample(ThreadContext &thread_context, const Scene &scene, const Camera &camera,
	uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t sample_index,
	float one_over_width, float one_over_height)
{
	const uint32_t image_index = thread_context.image_index;

	const float camera_x = (x + uniform(thread_context)) * one_over_width;
	const float camera_y = (y + uniform(thread_context)) * one_over_height;
	const Float3 camera_direction = generate_camera_direction(camera, camera_x, camera_y);

	IntersectResult intersect;
	if (!intersect_closest(scene, camera.position, camera_direction, intersect)) {
		if (image_index == 1)	return sky_color_in_direction(scene, camera_direction);
		return float3(0,0,0);
	}
	if (image_index == 0)
		return intersect.diffuse;
	else if (image_index == 1)
		return intersect.emissive;

	return float3(0,0,0);
}
