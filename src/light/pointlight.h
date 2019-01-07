#ifndef __GLUE__LIGHT__POINTLIGHT__
#define __GLUE__LIGHT__POINTLIGHT__

#include "light.h"

#include <glm/vec3.hpp>
#include <memory>

namespace glue
{
	namespace light
	{
		class Pointlight : public Light
		{
		public:
			//Xml structure of the class.
			struct Xml : public Light::Xml
			{
				glm::vec3 position;
				glm::vec3 flux;

				explicit Xml(const xml::Node& node);
				std::unique_ptr<Light> create() const override;
			};

		public:
			explicit Pointlight(const Pointlight::Xml& xml);

			Photon castPhoton(core::UniformSampler& sampler) const override { return Photon(); }
			LightSample sample(core::UniformSampler& sampler, const geometry::Intersection& intersection) const override;
			LightSample getVisibleSample(const core::Scene& scene, const geometry::Ray& ray) const override;
			glm::vec3 getLe(const glm::vec3& wi_world, const glm::vec3& light_plane_normal, float distance) const override;
			float getPdf(const glm::vec3& wi_world, const glm::vec3& light_plane_normal, float distance) const override;
			bool hasDeltaDistribution() const override;
			std::shared_ptr<geometry::Object> getObject() const override;

		private:
			glm::vec3 m_position;
			glm::vec3 m_intensity;
		};
	}
}

#endif