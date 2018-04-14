#ifndef __GLUE__CORE__SCENE__
#define __GLUE__CORE__SCENE__

#include "pinhole_camera.h"
#include "..\geometry\mesh.h"
#include "..\geometry\bvh.h"
#include "..\geometry\debug_sphere.h"
#include "..\light\light.h"

#include <glm\vec3.hpp>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

namespace glue
{
	namespace core
	{
		enum class Filter
		{
			BOX,
			GAUSSIAN
		};

		struct Scene
		{
			//Data
			std::vector<std::shared_ptr<geometry::Mesh>> meshes;
			geometry::BVH bvh;
			std::vector<std::shared_ptr<light::Light>> lights;
			std::unordered_map<const geometry::Mesh*, const light::Light*> light_meshes;
			std::vector<geometry::DebugSphere> debug_spheres;
			geometry::BVH debug_bvh;
			std::unique_ptr<PinholeCamera> camera;
			std::string image_name;
			glm::vec3 background_color;
			Filter pixel_filter;
			int sample_count;
			float secondary_ray_epsilon;

			//Functions
			void loadFromXML(const std::string& filepath);

			template<typename Integrator>
			void render(const Integrator& integrator);
		};
	}
}

#include "scene.inl"

#endif