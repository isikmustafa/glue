#include "dielectric.h"
#include "..\core\uniform_sampler.h"

namespace glue
{
	namespace material
	{
		Dielectric::Dielectric(float ior_n, float roughness)
			: m_ior_n(ior_n)
			, m_microfacet(roughness)
		{}

		std::pair<glm::vec3, glm::vec3> Dielectric::sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler) const
		{
			return m_microfacet.sampleWo(wi_tangent, sampler, material::cosTheta(wi_tangent) > 0.0f ? m_ior_n : 1.0f / m_ior_n);
		}

		glm::vec3 Dielectric::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const
		{
			return m_microfacet.getBsdf(wi_tangent, wo_tangent, material::cosTheta(wi_tangent) > 0.0f ? m_ior_n : 1.0f / m_ior_n);
		}

		float Dielectric::getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const
		{
			return m_microfacet.getPdf(wi_tangent, wo_tangent, material::cosTheta(wi_tangent) > 0.0f ? m_ior_n : 1.0f / m_ior_n);
		}

		bool Dielectric::hasDeltaDistribution() const
		{
			return false;
		}

		bool Dielectric::useMultipleImportanceSampling() const
		{
			return true;
		}
	}
}