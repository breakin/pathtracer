#include <embree2/rtcore.h>
#include <embree2/rtcore_scene.h>
#include <embree2/rtcore_geometry.h>
#include <embree2/rtcore_ray.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <algorithm>
#include "shared.h"
#include <vector>
#include <assert.h>

namespace {
	struct Material {
		Float3 diffuse, emissive;
	};
}

struct Scene {
	RTCScene embree_scene;
	Array<uint32_t> instance_material;
	Array<Material> materials;
};

bool intersect_closest(const Scene &scene, const Float3 pos, const Float3 dir, IntersectResult &out_result) {
	RTCRay ray;
	ray.org[0] = pos.x;
	ray.org[1] = pos.y;
	ray.org[2] = pos.z;
	ray.dir[0] = dir.x;
	ray.dir[1] = dir.y;
	ray.dir[2] = dir.z;
	ray.time = 0.0f;
	ray.tnear = 1E-5f;//0.0f;
	ray.mask = 0;
	ray.tfar = std::numeric_limits<float>::max();
	ray.instID = RTC_INVALID_GEOMETRY_ID;
	ray.geomID = RTC_INVALID_GEOMETRY_ID;
	ray.primID = RTC_INVALID_GEOMETRY_ID;
	rtcIntersect(scene.embree_scene, ray);

	if (ray.geomID == RTC_INVALID_GEOMETRY_ID)
		return false;

	const Material &m = scene.materials[scene.instance_material[ray.geomID]];
	out_result.diffuse = m.diffuse;
	out_result.emissive = m.emissive;
	out_result.pos = pos + dir * ray.tfar;
	Float3 fn = normalized(float3(ray.Ng[0], ray.Ng[1], ray.Ng[2]));
	if (dot(dir,fn)>0.0f)
		fn = -fn;

	out_result.face_normal = fn;
	return true;
}

namespace {

	/*
		NOTE: Anything in this file as a hack to support the post code.
			  It might be cleaned up over the course of the posts!
	*/

	void embree_error(void* userPtr, const RTCError code, const char* str) {
		printf("Embree error %s\n", str);
		exit(1);
	}

	//void post1(const Scene * const scene, Pixel &pixel, );

	struct Context {
		RTCDevice embree_device;
		Scene scene;
	};

	void add_cube(Scene &scene, uint32_t material_id, const Float3 center_pos, const Float3 size) {

		// TODO: Can we use rtcSetTransform on a non-instance? I think not.

		// TODO: Add normal that we can interpolate

		uint32_t mesh_id = scene.instance_material.size();
		scene.instance_material.push_back(material_id);

		// 6 quads with 4 vertices each = 6*4=24 vertices
		// This is because we want hard normals on our cube
		// Notice cute trick here; using material index as geometry index
		uint32_t embree_mesh_id = rtcNewQuadMesh2(scene.embree_scene, RTC_GEOMETRY_STATIC, 6, 24, 1, mesh_id);

		assert(mesh_id == embree_mesh_id);

		uint32_t *index_buffer = (uint32_t*)rtcMapBuffer(scene.embree_scene, mesh_id, RTC_INDEX_BUFFER);
		for (uint32_t i = 0; i < 6*4; ++i) {
			index_buffer[i] = i;
		}
		rtcUnmapBuffer(scene.embree_scene, mesh_id, RTC_INDEX_BUFFER);

		Float3 pos[8]={
			float3(-1,-1,-1),
			float3( 1,-1,-1),
			float3(-1,-1, 1),
			float3( 1,-1, 1),
			float3(-1, 1,-1),
			float3( 1, 1,-1),
			float3(-1, 1, 1),
			float3( 1, 1, 1),
		};

		for (uint32_t i=0; i<8; i++) {
			pos[i] = pos[i]*size + center_pos;
		}

		static const uint32_t idx[6][4]={
			{0,1,3,2}, // top
			{4,6,7,5}, // bottom
			{1,5,7,3}, // right
			{2,3,7,6}, // front
			{0,2,6,4}, // left
			{0,4,5,1},  // back
		};

		Float4 *vertex_buffer = (Float4*)rtcMapBuffer(scene.embree_scene, mesh_id, RTC_VERTEX_BUFFER);
		for (uint32_t f = 0, ofs = 0; f < 6; f++) {
			for (uint32_t v = 0; v < 4; v++, ofs++) {
				Float4 &vp = vertex_buffer[ofs];
				Float3 s = pos[idx[f][v]];
				vp.x = s.x;
				vp.y = s.y;
				vp.z = s.z;
				vp.w = 0.0f; // padding
			}
			
		}
		rtcUnmapBuffer(scene.embree_scene, mesh_id, RTC_VERTEX_BUFFER);
	}

	void create_scene(Context &context) {
		Scene &scene = context.scene;
		scene.embree_scene = rtcDeviceNewScene(context.embree_device, RTC_SCENE_STATIC|RTC_SCENE_INCOHERENT, RTC_INTERSECT1|RTC_INTERPOLATE);

		uint32_t red_material = scene.materials.size();
		scene.materials.push_back(Material{float3(1.0f,0.5f,0.5f), float3(0,0,0)});

		uint32_t white_material = scene.materials.size();
		scene.materials.push_back(Material{float3(0.9f,0.9f,0.9f), float3(0,0,0)});

		uint32_t red_emissive_material = scene.materials.size();
		scene.materials.push_back(Material{float3(0,0,0), float3(2.0f,0,0)});
		
		for (int x = -5; x<= 5; x++) {
			if (x==0)
				continue;
			add_cube(scene, red_material, float3(x*2.0f,2,0), float3(0.5f,0.5f,0.5f));
			add_cube(scene, red_material, float3(0,2,x*2.0f), float3(0.5f,0.5f,0.5f));
			add_cube(scene, red_material, float3(0,2+(5+x)*2.0f,0), float3(0.5f,0.5f,0.5f));
		}
		add_cube(scene, white_material, float3(0,-0.5f,0), float3(100.0f,0.5f,100.0f)); // Floor cube
		add_cube(scene, red_emissive_material, float3(0,0.5f,0), float3(1,1,1));

		rtcCommit(scene.embree_scene);
	}
}

// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
Float3 ACESFilm(Float3 x)
{
	float a = 2.51f;
    Float3 b = float3(0.03f, 0.03f, 0.03f);
    float c = 2.43f;
    Float3 d = float3(0.59f, 0.59f, 0.59f);
    Float3 e = float3(0.14f, 0.14f, 0.14f);

	Float3 t = (x*(a*x+b));
	Float3 n = (x*(c*x+d)+e);
	Float3 in = float3(1.0f/n.x, 1.0f/n.y, 1.0f/n.z);

    return (t*in); // TODO: Saturate
}

int main(void) {
	srand(1239);
	Context context;
	context.embree_device = rtcNewDevice();
	rtcDeviceSetErrorFunction2(context.embree_device, embree_error, &context);
	create_scene(context);

	const Scene &active_scene = context.scene;

	uint32_t width=640, height=480;
	std::vector<Pixel> framebuffer;
	framebuffer.resize(width*height);
	memset(&framebuffer[0], 0, sizeof(Pixel)*width*height);

	Camera camera;
	camera.position = float3(0,5,-10);
	camera.forward = float3(0,0,1);
	// TODO: Do aspect ratio at least
	camera.up = float3(0,-1,0); // TODO: Choose a coordinate system and act accordingly! -1 fixes that v value is upside down.. or is it?
	camera.right = float3(1,0,0);

	float du = 2.0f/width;
	float dv = 2.0f/height;

	// TODO: Multi-thread these loops. Use tile
	for (uint32_t y=0, ofs=0; y<height; y++) {
		float v = (y+0.5f) / (float)height;
		v = v * 2.0f - 1.0f;
		for (uint32_t x=0; x<width; x++, ofs++) {
			float u = (x+0.5f) / (float)width;
			u = u * 2.0f - 1.0f;
			Pixel &pixel = framebuffer[ofs];
			for (int k=0; k<256; k++) {
				Float3 color = pathtrace_pixel(active_scene, camera, x,y,width,height,u,v,du,dv);
				pixel.N++;
				pixel.rgb += (color-pixel.rgb) * (1.0f/pixel.N);
			}
		}
	}
	
	std::vector<uint32_t> byte_data(width*height);
	for (uint32_t y=0, ofs=0; y<height; y++) {
		for (uint32_t x=0; x<width; x++, ofs++) {
			Float3 result = ACESFilm(framebuffer[ofs].rgb);
			uint8_t r8 = (uint8_t)std::min(round(result.x * 255.0f), 255.0f);
			uint8_t g8 = (uint8_t)std::min(round(result.y * 255.0f), 255.0f);
			uint8_t b8 = (uint8_t)std::min(round(result.z * 255.0f), 255.0f);
			uint8_t a8 = 0xFF;
			uint32_t c = r8|(g8<<8)|(b8<<16)|(a8<<24);
			byte_data[ofs] = c;
		}
	}

	stbi_write_png("output.png", width, height, 4, (const void*)&byte_data[0], 0);

	rtcDeleteScene(context.scene.embree_scene);
	rtcDeleteDevice(context.embree_device);
	return 0;
}
