#ifndef __GLUE__CORE__SCENE__
#define __GLUE__CORE__SCENE__

#include "pinhole_camera.h"
#include "image.h"
#include "output.h"
#include "..\geometry\object.h"
#include "..\geometry\bvh.h"
#include "..\light\light.h"
#include "..\integrator\integrator.h"

#include <vector>
#include <memory>
#include <unordered_map>
#include <glm\vec3.hpp>

namespace glue
{
	namespace core
	{
		struct Scene
		{
		public:
			//Xml structure of the class.
			struct Xml
			{
				glm::vec3 background_radiance;
				float secondary_ray_epsilon;
				std::unique_ptr<integrator::Integrator::Xml> integrator;
				std::vector<std::unique_ptr<Output::Xml>> outputs;
				std::unique_ptr<PinholeCamera::Xml> camera;
				std::vector<std::unique_ptr<geometry::Object::Xml>> objects;
				std::vector<std::unique_ptr<light::Light::Xml>> lights;

				explicit Xml(const xml::Node& node);
			};

		public:
			std::unique_ptr<PinholeCamera> camera;
			geometry::BVH<std::shared_ptr<geometry::Object>> bvh;
			std::vector<std::shared_ptr<light::Light>> lights;
			std::shared_ptr<light::Light> environment_light;
			std::unordered_map<const geometry::Object*, const light::Light*> object_to_light;
			glm::vec3 background_radiance;
			float secondary_ray_epsilon;

		public:
			Scene(const Scene::Xml& xml);

			void render();
			glm::vec3 getBackgroundRadiance(const glm::vec3& direction, bool light_explicitly_sampled) const;

		private:
			std::unique_ptr<integrator::Integrator> m_integrator;
			std::unique_ptr<Image> m_image;
			std::vector<std::unique_ptr<Output>> m_outputs;
		};
	}
}

#endif