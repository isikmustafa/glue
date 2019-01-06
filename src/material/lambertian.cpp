#include "lambertian.h"
#include "../geometry/spherical_coordinate.h"
#include "../core/real_sampler.h"
#include "../core/math.h"
#include "../xml/node.h"

#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>

namespace glue
{
	namespace material
	{
		Lambertian::Xml::Xml(const xml::Node& node)
		{
			kd = texture::Texture::Xml::factory(node.child("Kd", true));
		}

		Lambertian::Xml::Xml(std::unique_ptr<texture::Texture::Xml> p_kd)
			: kd(std::move(p_kd))
		{}

		std::unique_ptr<BsdfMaterial> Lambertian::Xml::create() const
		{
			return std::make_unique<Lambertian>(*this);
		}

		Lambertian::Lambertian(const Lambertian::Xml& xml)
			: m_kd(xml.kd->create())
		{}

		std::pair<glm::vec3, glm::vec3> Lambertian::sampleWi(const glm::vec3& wo_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const
		{
			glm::vec3 wi(0.0f);
			glm::vec3 f(0.0f);

			if (core::math::cosTheta(wo_tangent) > 0.0f)
			{
				wi = core::math::sampleHemisphereCosine(sampler.sample(), sampler.sample()).toCartesianCoordinate();
				f = m_kd->fetch(intersection);
			}

			return std::make_pair(wi, f);
		}

		glm::vec3 Lambertian::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const
		{
			if (core::math::cosTheta(wi_tangent) > 0.0f && core::math::cosTheta(wo_tangent) > 0.0f)
			{
				return m_kd->fetch(intersection) * glm::one_over_pi<float>();
			}

			return glm::vec3(0.0f);
		}

		float Lambertian::getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const
		{
			if (core::math::cosTheta(wi_tangent) > 0.0f && core::math::cosTheta(wo_tangent) > 0.0f)
			{
				return core::math::cosTheta(wi_tangent) * glm::one_over_pi<float>();
			}

			return 0.0f;
		}

		bool Lambertian::hasDeltaDistribution(const geometry::Intersection& intersection) const
		{
			return false;
		}

		bool Lambertian::useMultipleImportanceSampling(const geometry::Intersection& intersection) const
		{
			return false;
		}

		bool Lambertian::isSpecular(const geometry::Intersection& intersection) const
		{
			return false;
		}
	}
}