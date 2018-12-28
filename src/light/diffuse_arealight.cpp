#include "diffuse_arealight.h"
#include "../core/real_sampler.h"
#include "../core/scene.h"
#include "../xml/node.h"

#include <glm/gtc/constants.hpp>

namespace glue
{
	namespace light
	{
		DiffuseArealight::Xml::Xml(const xml::Node& node)
			: object(geometry::Object::Xml::factory(node.child("Object", true)))
		{
			node.parseChildText("Flux", &flux.x, &flux.y, &flux.z);
			attributes = node.attributes();
		}

		std::unique_ptr<Light> DiffuseArealight::Xml::create() const
		{
			return std::make_unique<DiffuseArealight>(*this);
		}

		DiffuseArealight::DiffuseArealight(const DiffuseArealight::Xml& xml)
			: m_object(xml.object->create())
			, m_pdf(1.0f / m_object->getSurfaceArea())
			, m_le(xml.flux * glm::one_over_pi<float>() * m_pdf)
		{}

		LightSample DiffuseArealight::sample(core::UniformSampler& sampler, const geometry::Intersection& intersection) const
		{
			LightSample light_sample;

			auto plane = m_object->samplePlane(sampler);
			light_sample.wi_world = plane.point - intersection.plane.point;
			light_sample.distance = glm::length(light_sample.wi_world);
			light_sample.wi_world /= light_sample.distance;
			light_sample.le = getLe(light_sample.wi_world, plane.normal, light_sample.distance);
			light_sample.pdf_w = getPdf(light_sample.wi_world, plane.normal, light_sample.distance);

			return light_sample;
		}

		LightSample DiffuseArealight::getVisibleSample(const core::Scene& scene, const geometry::Ray& ray) const
		{
			light::LightSample light_sample;
			geometry::Intersection intersection;
			if (scene.bvh.intersect(ray, intersection, std::numeric_limits<float>::max()))
			{
				//If ray through sampled direction hits this light, add its contribution.
				auto itr = scene.object_to_light.find(intersection.object);
				if (itr != scene.object_to_light.end() && itr->second == this)
				{
					intersection.object->fillIntersection(ray, intersection);

					light_sample.wi_world = ray.get_direction();
					light_sample.le = getLe(light_sample.wi_world, intersection.plane.normal, intersection.distance);
					light_sample.pdf_w = getPdf(light_sample.wi_world, intersection.plane.normal, intersection.distance);
					light_sample.distance = intersection.distance;
				}
			}

			return light_sample;
		}

		glm::vec3 DiffuseArealight::getLe(const glm::vec3& wi_world, const glm::vec3& light_plane_normal, float distance) const
		{
			return glm::dot(-wi_world, light_plane_normal) > 0.0f ? m_le : glm::vec3(0.0f);
		}

		float DiffuseArealight::getPdf(const glm::vec3& wi_world, const glm::vec3& light_plane_normal, float distance) const
		{
			//Transform p(A) to p(w)
			return m_pdf * distance * distance / glm::abs(glm::dot(-wi_world, light_plane_normal));
		}

		bool DiffuseArealight::hasDeltaDistribution() const
		{
			return false;
		}

		std::shared_ptr<geometry::Object> DiffuseArealight::getObject() const
		{
			return m_object;
		}
	}
}