#ifndef __GLUE__LIGHT__DIFFUSEAREALIGHT__
#define __GLUE__LIGHT__DIFFUSEAREALIGHT__

#include "light.h"

#include <glm/vec3.hpp>
#include <memory>

namespace glue
{
	namespace light
	{
		class DiffuseArealight : public Light
		{
		public:
			//Xml structure of the class.
			struct Xml : public Light::Xml
			{
				glm::vec3 flux;
				std::unique_ptr<geometry::Object::Xml> object;

				explicit Xml(const xml::Node& node);
				std::unique_ptr<Light> create() const override;
			};

		public:
			explicit DiffuseArealight(const DiffuseArealight::Xml& xml);

			LightSample sample(core::UniformSampler& sampler, const geometry::Intersection& intersection) const override;
			LightSample getVisibleSample(const core::Scene& scene, const geometry::Ray& ray) const override;
			glm::vec3 getLe(const glm::vec3& wi_world, const glm::vec3& light_plane_normal, float distance) const override;
			float getPdf(const glm::vec3& wi_world, const glm::vec3& light_plane_normal, float distance) const override;
			bool hasDeltaDistribution() const override;
			std::shared_ptr<geometry::Object> getObject() const override;

		private:
			std::shared_ptr<geometry::Object> m_object;
			float m_pdf;
			glm::vec3 m_le;
		};
	}
}

#endif