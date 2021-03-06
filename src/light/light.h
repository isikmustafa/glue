#ifndef __GLUE__LIGHT__LIGHT__
#define __GLUE__LIGHT__LIGHT__

#include "../geometry/plane.h"
#include "../geometry/object.h"
#include "../geometry/ray.h"
#include "../core/forward_decl.h"

#include <glm/vec3.hpp>
#include <memory>
#include <unordered_map>

namespace glue
{
	namespace light
	{
		struct LightSample
		{
			glm::vec3 wi_world;
			glm::vec3 le{0.0f, 0.0f, 0.0f};
			float pdf_w{0.0f};
			float distance;
		};

		struct Photon
		{
			geometry::Ray ray; //Origin and direction of photon.
			glm::vec3 beta; //Throughput of photon.

			Photon()=default;
			Photon(const geometry::Ray& p_ray, const glm::vec3& p_beta)
				: ray(p_ray)
				, beta(p_beta)
			{}
		};

		class Light
		{
		public:
			//Xml structure of the class.
			struct Xml
			{
				std::unordered_map<std::string, std::string> attributes;

				virtual ~Xml() = default;
				virtual std::unique_ptr<Light> create() const = 0;
				static std::unique_ptr<Light::Xml> factory(const xml::Node& node);
			};

		public:
			virtual ~Light() = default;

			virtual Photon castPhoton(core::UniformSampler& sampler) const = 0;
			virtual LightSample sample(core::UniformSampler& sampler, const geometry::Intersection& intersection) const = 0;
			virtual LightSample getVisibleSample(const core::Scene& scene, const geometry::Ray& ray) const = 0;
			virtual glm::vec3 getLe(const glm::vec3& wi_world, const glm::vec3& light_plane_normal, float distance) const = 0;
			virtual float getPdf(const glm::vec3& wi_world, const glm::vec3& light_plane_normal, float distance) const = 0;
			virtual bool hasDeltaDistribution() const = 0;
			virtual std::shared_ptr<geometry::Object> getObject() const = 0;
		};
	}
}

#endif