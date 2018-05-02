#include "ggx_distribution.h"
#include "..\material\bsdf_material.h"
#include "..\geometry\spherical_coordinate.h"

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

		glm::vec3 GGXDistribution::sampleWh(core::UniformSampler& sampler) const
		{
			//Sample from d(wh)
			auto u = sampler.sample();
			return geometry::SphericalCoordinate(1.0f,
				glm::atan(m_ag * glm::sqrt(u) / glm::sqrt(1 - u)),
				glm::two_pi<float>() * sampler.sample()).toCartesianCoordinate();
		}

		float GGXDistribution::d(const glm::vec3& wh_tangent) const
		{
			auto costheta_h = material::cosTheta(wh_tangent);
			if (costheta_h > 0.0f)
			{
				auto temp = m_ag * m_ag + material::tan2Theta(wh_tangent);
				return m_ag * m_ag / (glm::pi<float>() * costheta_h * costheta_h * costheta_h * costheta_h * temp * temp);
			}

			return 0.0f;
		}

		float GGXDistribution::g1(const glm::vec3& wv_tangent, const glm::vec3& wh_tangent) const
		{
			if (glm::dot(wv_tangent, wh_tangent) / material::cosTheta(wv_tangent) > 0.0f)
			{
				auto tantheta_v = material::tanTheta(wv_tangent);
				return 2.0f / (1.0f + glm::sqrt(1.0f + m_ag * m_ag * tantheta_v * tantheta_v));
			}

			return 0.0f;
		}
	}
}