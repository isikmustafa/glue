#include "oren_nayar.h"
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
		OrenNayar::Xml::Xml(const xml::Node& node)
		{
			kd = texture::Texture::Xml::factory(node.child("Kd", true));
			node.parseChildText("Roughness", &roughness);
		}

		OrenNayar::Xml::Xml(std::unique_ptr<texture::Texture::Xml> p_kd, float p_roughness)
			: kd(std::move(p_kd))
			, roughness(p_roughness)
		{}

		std::unique_ptr<BsdfMaterial> OrenNayar::Xml::create() const
		{
			return std::make_unique<OrenNayar>(*this);
		}

		OrenNayar::OrenNayar(const OrenNayar::Xml& xml)
			: m_kd(xml.kd->create())
		{
			auto r2 = xml.roughness * xml.roughness;
			m_A = 1.0f - r2 / (2.0f * (r2 + 0.33f));
			m_B = 0.45f * r2 / (r2 + 0.09f);
		}

		std::pair<glm::vec3, glm::vec3> OrenNayar::sampleWi(const glm::vec3& wo_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const
		{
			glm::vec3 wi(0.0f);
			glm::vec3 f(0.0f);

			if (core::math::cosTheta(wo_tangent) > 0.0f)
			{
				wi = geometry::SphericalCoordinate(1.0f, glm::acos(glm::sqrt(sampler.sample())), glm::two_pi<float>() * sampler.sample()).toCartesianCoordinate();
				f = getBsdf(wi, wo_tangent, intersection) * glm::pi<float>();
			}

			return std::make_pair(wi, f);
		}

		glm::vec3 OrenNayar::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const
		{
			if (core::math::cosTheta(wi_tangent) > 0.0f && core::math::cosTheta(wo_tangent) > 0.0f)
			{
				geometry::SphericalCoordinate wi_ts(wi_tangent);
				geometry::SphericalCoordinate wo_ts(wo_tangent);

				return m_kd->fetch(intersection) * glm::one_over_pi<float>() *
				        (m_A + m_B * glm::max(0.0f, glm::cos(wi_ts.phi - wo_ts.phi)) *
				        glm::sin(glm::max(wi_ts.theta, wo_ts.theta)) * glm::tan(glm::min(wi_ts.theta, wo_ts.theta)));
			}

			return glm::vec3(0.0f);
		}

		float OrenNayar::getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const
		{
			if (core::math::cosTheta(wi_tangent) > 0.0f && core::math::cosTheta(wo_tangent) > 0.0f)
			{
				return core::math::cosTheta(wi_tangent) * glm::one_over_pi<float>();
			}

			return 0.0f;
		}

		bool OrenNayar::hasDeltaDistribution(const geometry::Intersection& intersection) const
		{
			return false;
		}

		bool OrenNayar::useMultipleImportanceSampling(const geometry::Intersection& intersection) const
		{
			return false;
		}
	}
}