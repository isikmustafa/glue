#ifndef __GLUE__LIGHT__LIGHT__
#define __GLUE__LIGHT__LIGHT__

#include "..\geometry\plane.h"
#include "..\geometry\object.h"
#include "..\core\forward_decl.h"

#include <glm\vec3.hpp>
#include <memory>

namespace glue
{
	namespace light
	{
		struct LightSample
		{
			glm::vec3 wo_world;
			glm::vec3 le;
			float pdf_w;
			float distance;
		};

		class Light
		{
		public:
			//Xml structure of the class.
			struct Xml
			{
				virtual ~Xml() {}
				virtual std::unique_ptr<Light> create() const = 0;
				static std::unique_ptr<Light::Xml> factory(const xml::Node& node);
			};

		public:
			virtual ~Light() {}

			virtual LightSample sample(core::UniformSampler& sampler, const geometry::Intersection& intersection) const = 0;
			virtual LightSample getVisibleSample(const core::Scene& scene, const geometry::Ray& ray) const = 0;
			virtual glm::vec3 getLe(const glm::vec3& wo_world, const glm::vec3& light_plane_normal, float distance) const = 0;
			virtual float getPdf(const glm::vec3& wo_world, const glm::vec3& light_plane_normal, float distance) const = 0;
			virtual bool hasDeltaDistribution() const = 0;
			virtual std::shared_ptr<geometry::Object> getObject() const = 0;
		};
	}
}

#endif