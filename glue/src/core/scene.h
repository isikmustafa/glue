#ifndef __GLUE__CORE__SCENE__
#define __GLUE__CORE__SCENE__

#include "camera.h"
#include "..\geometry\mesh.h"
#include "..\geometry\bvh.h"

#include <glm\vec3.hpp>
#include <vector>
#include <string>

namespace glue
{
	namespace core
	{
		struct Scene
		{
			//Data
			std::vector<geometry::Mesh> meshes;
			geometry::BVH bvh;
			Camera camera;
			std::string image_name;
			glm::vec3 background_color;
			std::string pixel_filter;
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