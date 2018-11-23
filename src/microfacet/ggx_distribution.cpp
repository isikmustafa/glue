#include "ggx_distribution.h"
#include "../geometry/spherical_coordinate.h"
#include "../core/real_sampler.h"
#include "../core/math.h"

#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>

namespace glue
{
	namespace microfacet
	{
		GGXDistribution::GGXDistribution(float roughness)
			: m_ag(roughness)
		{}

		glm::vec3 GGXDistribution::sampleWhWmlt07(core::UniformSampler& sampler) const
		{
			auto u = sampler.sample();
			return geometry::SphericalCoordinate(1.0f,
				glm::atan(m_ag * glm::sqrt(u) / glm::sqrt(1.0f - u)),
				glm::two_pi<float>() * sampler.sample()).toCartesianCoordinate();
		}

		float GGXDistribution::pdfWmlt07(const glm::vec3& wh_tangent) const
		{
			return glm::abs(d(wh_tangent) * core::math::cosTheta(wh_tangent));
		}

		glm::vec3 GGXDistribution::sampleWhHd14(const glm::vec3& wi_tangent, core::UniformSampler& sampler) const
		{
			auto wi = core::math::cosTheta(wi_tangent) < 0.0f ? -wi_tangent : wi_tangent;

			//A simpler and exact sampling routine for GGX.
			//Eric Heitz. A Simpler and Exact Sampling Routine for the GGX Distribution of Visible Normals. 2017

			//Stretch
			auto stretched_wi = glm::normalize(glm::vec3(m_ag * wi.x, m_ag * wi.y, wi.z));

			//Create an orthonormal basis using stretched_wi.
			auto t1 = core::math::cosTheta(stretched_wi) < 0.9999f ? glm::normalize(glm::cross(stretched_wi, glm::vec3(0.0f, 0.0f, 1.0f))) : glm::vec3(1.0f, 0.0f, 0.0f);
			auto t2 = glm::cross(t1, stretched_wi);
			//Sample a point from one of the half disks.
			auto u1 = sampler.sample();
			auto u2 = sampler.sample();
			auto a = 1.0f / (1.0f + core::math::cosTheta(stretched_wi));
			auto r = glm::sqrt(u1);
			auto phi = u2 < a ? u2 / a * glm::pi<float>() : glm::pi<float>() + (u2 - a) / (1.0f - a) * glm::pi<float>();
			auto p1 = r * glm::cos(phi);
			auto p2 = r * glm::sin(phi) * (u2 < a ? 1.0f : core::math::cosTheta(stretched_wi));
			auto p3 = glm::sqrt(glm::max(0.0f, 1.0f - p1 * p1 - p2 * p2));
			//Compute the normal.
			auto n = p1 * t1 + p2 * t2 + p3 * stretched_wi;

			//Unstretch
			return glm::normalize(glm::vec3(m_ag * n.x, m_ag * n.y, glm::max(0.0f, n.z)));
		}

		float GGXDistribution::pdfHd14(const glm::vec3& wi_tangent, const glm::vec3& wh_tangent) const
		{
			return glm::abs(g1(wi_tangent, wh_tangent) * glm::dot(wi_tangent, wh_tangent) * d(wh_tangent) / core::math::cosTheta(wi_tangent));
		}

		float GGXDistribution::d(const glm::vec3& wh_tangent) const
		{
			auto costheta_h = core::math::cosTheta(wh_tangent);
			if (costheta_h > 0.0f)
			{
				auto temp = m_ag * m_ag + core::math::tan2Theta(wh_tangent);
				return m_ag * m_ag / (glm::pi<float>() * costheta_h * costheta_h * costheta_h * costheta_h * temp * temp);
			}

			return 0.0f;
		}

		float GGXDistribution::g1(const glm::vec3& wv_tangent, const glm::vec3& wh_tangent) const
		{
			if (glm::dot(wv_tangent, wh_tangent) / core::math::cosTheta(wv_tangent) > 0.0f)
			{
				return 2.0f / (1.0f + glm::sqrt(1.0f + m_ag * m_ag * core::math::tan2Theta(wv_tangent)));
			}

			return 0.0f;
		}
	}
}