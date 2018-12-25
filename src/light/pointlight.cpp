#include "pointlight.h"
#include "../geometry/intersection.h"
#include "../xml/node.h"

#include <glm/gtc/constants.hpp>
#include <glm/geometric.hpp>

namespace glue
{
	namespace light
	{
		Pointlight::Xml::Xml(const xml::Node& node)
		{
			attributes = node.attributes();
			node.parseChildText("Position", &position.x, &position.y, &position.z);
			node.parseChildText("Flux", &flux.x, &flux.y, &flux.z);
		}

		std::unique_ptr<Light> Pointlight::Xml::create() const
		{
			return std::make_unique<Pointlight>(*this);
		}

		Pointlight::Pointlight(const Pointlight::Xml& xml)
			: m_position(xml.position)
			, m_intensity(xml.flux / (4.0f * glm::pi<float>()))
		{}

		LightSample Pointlight::sample(core::UniformSampler& sampler, const geometry::Intersection& intersection) const
		{
			LightSample light_sample;

			light_sample.wo_world = m_position - intersection.plane.point;
			light_sample.distance = glm::length(light_sample.wo_world);
			light_sample.wo_world /= light_sample.distance;
			light_sample.le = m_intensity / (light_sample.distance * light_sample.distance);
			light_sample.pdf_w = 1.0f;

			return light_sample;
		}

		LightSample Pointlight::getVisibleSample(const core::Scene& scene, const geometry::Ray& ray) const
		{
			return light::LightSample();
		}

		glm::vec3 Pointlight::getLe(const glm::vec3& wo_world, const glm::vec3& light_plane_normal, float distance) const
		{
			return m_intensity / (distance * distance);
		}

		float Pointlight::getPdf(const glm::vec3& wo_world, const glm::vec3& light_plane_normal, float distance) const
		{
			return 1.0f;
		}

		bool Pointlight::hasDeltaDistribution() const
		{
			return true;
		}

		std::shared_ptr<geometry::Object> Pointlight::getObject() const
		{
			return nullptr;
		}
	}
}