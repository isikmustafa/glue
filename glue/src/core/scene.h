#ifndef __GLUE__CORE__SCENE__
#define __GLUE__CORE__SCENE__

#include "camera.h"
#include "..\geometry\mesh.h"
#include "..\geometry\bvh.h"
#include "..\light\light.h"

#include <glm\vec3.hpp>
#include <vector>
#include <unordered_map>
#include <string>

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
			std::vector<geometry::Mesh> meshes;
			std::vector<light::Light> lights;
			std::unordered_map<geometry::Mesh*, light::Light*> mesh_to_light;
			geometry::BVH bvh;
			Camera camera;
			std::string image_name;
			glm::vec3 background_color;
			Filter pixel_filter;
			int sample_count;

			//Functions
			void loadFromXML(const std::string& filepath);

			template<typename Integrator>
			void render(const Integrator& integrator);
		};
	}
}

#include "scene.inl"

#endif