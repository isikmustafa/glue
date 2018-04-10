#ifndef __GLUE__CORE__SCENE__
#define __GLUE__CORE__SCENE__

#include "camera.h"
#include "..\geometry\mesh.h"
#include "..\geometry\bvh.h"
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