#include "beckmann_distribution.h"
#include "..\material\bsdf_material.h"
#include "..\geometry\spherical_coordinate.h"

#include <glm\gtc\constants.hpp>
#include <glm\geometric.hpp>
#include <glm\trigonometric.hpp>

namespace glue
{
	namespace microfacet
	{
		BeckmannDistribution::BeckmannDistribution(float roughness)
			: m_ab(glm::pow(0.01f + 0.99f * roughness, 2.0f))
		{}

		glm::vec3 BeckmannDistribution::sampleWh(core::UniformSampler& sampler) const
		{
			//Sample from d(wh)
			return geometry::SphericalCoordinate(1.0f,
				glm::atan(glm::sqrt(-m_ab * m_ab * glm::log(1.0f - sampler.sample()))),
				glm::two_pi<float>() * sampler.sample()).toCartesianCoordinate();
		}

		float BeckmannDistribution::d(const glm::vec3& wh_tangent) const
		{
			auto costheta_h = material::cosTheta(wh_tangent);
			if (costheta_h > 0.0f)
			{
				auto pow = -material::tan2Theta(wh_tangent) / (m_ab * m_ab);
				return glm::exp(pow) / (glm::pi<float>() * m_ab * m_ab * costheta_h * costheta_h * costheta_h * costheta_h);
			}

			return 0.0f;
		}

		float BeckmannDistribution::g1(const glm::vec3& wv_tangent, const glm::vec3& wh_tangent) const
		{
			if (glm::dot(wv_tangent, wh_tangent) / material::cosTheta(wv_tangent) > 0.0f)
			{
				auto a = 1.0f / (m_ab * material::tanTheta(wv_tangent));
				auto a2 = a * a;
				return a < 1.6f ? (3.535f * a + 2.181f * a2) / (1.0f + 2.276f * a + 2.577f * a2) : 1.0f;
			}

			return 0.0f;
		}
	}
}