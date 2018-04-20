#include "oren_nayar.h"
#include "..\geometry\spherical_coordinate.h"

#include <glm\gtc\constants.hpp>
#include <glm\geometric.hpp>
#include <glm\trigonometric.hpp>

namespace glue
{
	namespace material
	{
		OrenNayar::OrenNayar(const glm::vec3& kd, float roughness)
			: m_kd(kd * glm::one_over_pi<float>())
		{
			auto r2 = roughness * roughness;
			m_A = 1.0f - r2 / (2.0f * (r2 + 0.33f));
			m_B = 0.45f * r2 / (r2 + 0.09f);
		}

		glm::vec3 OrenNayar::sampleWi(const glm::vec3& wo_tangent, core::UniformSampler& sampler) const
		{
			//Sample from cosine-weighted distribution.
			return geometry::SphericalCoordinate(1.0f, glm::acos(glm::sqrt(sampler.sample())), glm::two_pi<float>() * sampler.sample()).toCartesianCoordinate();
		}

		glm::vec3 OrenNayar::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const
		{
			geometry::SphericalCoordinate wi_ts(wi_tangent);
			geometry::SphericalCoordinate wo_ts(wo_tangent);

			return m_kd * (m_A + m_B * glm::max(0.0f, glm::cos(wi_ts.phi - wo_ts.phi)) * glm::sin(glm::max(wi_ts.theta, wo_ts.theta)) *
				glm::tan(glm::min(wi_ts.theta, wo_ts.theta)));
		}

		float OrenNayar::getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const
		{
			//Cosine-weighted pdf.
			return cosTheta(wi_tangent) * glm::one_over_pi<float>();
		}

		bool OrenNayar::hasDeltaDistribution() const
		{
			return false;
		}

		bool OrenNayar::useMultipleImportanceSampling() const
		{
			return false;
		}
	}
}