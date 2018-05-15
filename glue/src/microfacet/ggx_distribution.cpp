#include "ggx_distribution.h"
#include "..\geometry\spherical_coordinate.h"
#include "..\core\real_sampler.h"
#include "..\core\math.h"

#include <glm\gtc\constants.hpp>
#include <glm\geometric.hpp>
#include <glm\trigonometric.hpp>

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

			//1-Stretch
			auto stretched_wi = glm::normalize(glm::vec3(m_ag * wi.x, m_ag * wi.y, wi.z));
			geometry::SphericalCoordinate spherical_stretched_wi(1.0f, 0.0f, 0.0f);
			
			if (core::math::cosTheta(stretched_wi) < 0.99999f)
			{
				spherical_stretched_wi = geometry::SphericalCoordinate(stretched_wi);
			}

			//2-Sample P22
			glm::vec2 slope;
			auto u1 = sampler.sample();
			auto u2 = sampler.sample();
			if (spherical_stretched_wi.theta > 0.0001f)
			{
				auto tantheta_wi = glm::tan(spherical_stretched_wi.theta);
				auto g1 = 2.0f / (1.0f + glm::sqrt(1.0f + tantheta_wi * tantheta_wi));

				//Sample slope.x
				auto A = 2.0f * u1 / g1 - 1.0f;
				auto temp = 1.0f / (A * A - 1.0f);
				auto B = tantheta_wi;
				auto D = glm::sqrt(B * B * temp * temp - (A * A - B * B) * temp);
				auto slope_x1 = B * temp - D;
				auto slope_x2 = B * temp + D;
				slope.x = (A < 0.0f || slope_x2 > 1.0f / tantheta_wi) ? slope_x1 : slope_x2;

				//Sample slope.y
				auto s = u2 <= 0.5f ? -1.0f : 1.0f;
				u2 = 2.0f * (u2 <= 0.5f ? (0.5f - u2) : (u2 - 0.5f));

				auto z = (u2 * (u2 * (u2 * 0.27385f - 0.73369f) + 0.46341f)) / (u2 * (u2 * (u2 * 0.093073f + 0.309420f) - 1.0f) + 0.597999f);
				slope.y = s * z * glm::sqrt(1.0f + slope.x * slope.x);
			}
			//Normal incidence case.
			else
			{
				auto r = glm::sqrt(u1 / (1.0f - u1));
				auto phi = glm::two_pi<float>() * u2;
				slope = r * glm::vec2(glm::cos(phi), glm::sin(phi));
			}

			//3-Rotate
			auto sin_phi = glm::sin(spherical_stretched_wi.phi);
			auto cos_phi = glm::cos(spherical_stretched_wi.phi);
			slope = glm::vec2(cos_phi * slope.x - sin_phi * slope.y, sin_phi * slope.x + cos_phi * slope.y);

			//4-Unstretch
			slope *= m_ag;

			//5-Compute normal
			return glm::normalize(glm::vec3(-slope, 1.0f));
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