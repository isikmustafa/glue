#include "beckmann_distribution.h"
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
		BeckmannDistribution::BeckmannDistribution(float roughness)
			: m_ab(roughness)
		{}

		glm::vec3 BeckmannDistribution::sampleWhWmlt07(core::UniformSampler& sampler) const
		{
			return geometry::SphericalCoordinate(1.0f,
				glm::atan(glm::sqrt(-m_ab * m_ab * glm::log(1.0f - sampler.sample()))),
				glm::two_pi<float>() * sampler.sample()).toCartesianCoordinate();
		}

		float BeckmannDistribution::pdfWmlt07(const glm::vec3& wh_tangent) const
		{
			return glm::abs(d(wh_tangent) * core::math::cosTheta(wh_tangent));
		}

		glm::vec3 BeckmannDistribution::sampleWhHd14(const glm::vec3& wi_tangent, core::UniformSampler& sampler) const
		{
			auto wi = core::math::cosTheta(wi_tangent) < 0.0f ? -wi_tangent : wi_tangent;

			//1-Stretch
			auto stretched_wi = glm::normalize(glm::vec3(m_ab * wi.x, m_ab * wi.y, wi.z));
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
				auto sintheta_wi = glm::sin(spherical_stretched_wi.theta);
				auto costheta_wi = glm::cos(spherical_stretched_wi.theta);
				auto tantheta_wi = sintheta_wi / costheta_wi;
				auto a = 1.0f / tantheta_wi;
				auto erf_a = std::erf(a);
				auto exp_a2 = glm::exp(-a * a);
				auto sqrt_pi_inv = 0.56418958354f;
				auto lambda = 0.5f * (erf_a - 1.0f) + 0.5f * sqrt_pi_inv * exp_a2 / a;
				auto g1 = 1.0f / (1.0f + lambda);
				auto C = 1.0f - g1 * erf_a;

				//Sample slope.x
				if (u1 < C)
				{
					u1 = u1 / C; //Reuse u1.

					auto w_1 = 0.5f * sqrt_pi_inv * sintheta_wi * exp_a2;
					auto w_2 = costheta_wi * (0.5f - 0.5f * erf_a);
					auto p = w_1 / (w_1 + w_2);
					if (u1 < p)
					{
						u1 = u1 / p; //Reuse u1.
						slope.x = -glm::sqrt(-glm::log(u1 * exp_a2));
					}
					else
					{
						u1 = (u1 - p) / (1.0f - p); //Reuse u1.
						slope.x = core::math::inverseErf(u1 - 1.0f - u1 * erf_a);
					}
				}
				else
				{
					u1 = (u1 - C) / (1.0f - C); //Reuse u1.

					slope.x = core::math::inverseErf((-1.0f + 2.0f * u1) * erf_a);
					auto p = (-slope.x * sintheta_wi + costheta_wi) / (2.0f * costheta_wi);
					if (u2 > p)
					{
						slope.x = -slope.x;
						u2 = (u2 - p) / (1.0f - p); //Reuse u2.
					}
					else
					{
						u2 = u2 / p; //Reuse u2.
					}
				}

				//Sample slope.y
				slope.y = core::math::inverseErf(2.0f * u2 - 1.0f);
			}
			//Normal incidence case.
			else
			{
				auto r = glm::sqrt(-glm::log(u1));
				auto phi = glm::two_pi<float>() * u2;
				slope = r * glm::vec2(glm::cos(phi), glm::sin(phi));
			}

			//3-Rotate
			auto sin_phi = glm::sin(spherical_stretched_wi.phi);
			auto cos_phi = glm::cos(spherical_stretched_wi.phi);
			slope = glm::vec2(cos_phi * slope.x - sin_phi * slope.y, sin_phi * slope.x + cos_phi * slope.y);

			//4-Unstretch
			slope *= m_ab;

			//5-Compute normal
			return glm::normalize(glm::vec3(-slope, 1.0f));
		}

		float BeckmannDistribution::pdfHd14(const glm::vec3& wi_tangent, const glm::vec3& wh_tangent) const
		{
			return glm::abs(g1(wi_tangent, wh_tangent) * glm::dot(wi_tangent, wh_tangent) * d(wh_tangent) / core::math::cosTheta(wi_tangent));
		}

		float BeckmannDistribution::d(const glm::vec3& wh_tangent) const
		{
			auto costheta_h = core::math::cosTheta(wh_tangent);
			if (costheta_h > 0.0f)
			{
				auto pow = -core::math::tan2Theta(wh_tangent) / (m_ab * m_ab);
				return glm::exp(pow) / (glm::pi<float>() * m_ab * m_ab * costheta_h * costheta_h * costheta_h * costheta_h);
			}

			return 0.0f;
		}

		float BeckmannDistribution::g1(const glm::vec3& wv_tangent, const glm::vec3& wh_tangent) const
		{
			if (glm::dot(wv_tangent, wh_tangent) / core::math::cosTheta(wv_tangent) > 0.0f)
			{
				auto a = 1.0f / (m_ab * core::math::tanTheta(wv_tangent));
				auto a2 = a * a;
				return a < 1.6f ? (3.535f * a + 2.181f * a2) / (1.0f + 2.276f * a + 2.577f * a2) : 1.0f;
			}

			return 0.0f;
		}
	}
}