#include "metal.h"

namespace glue
{
	namespace material
	{
		Metal::Metal(const glm::vec3& n_ratio, const glm::vec3& k_ratio, float roughness)
			: m_microfacet(n_ratio, k_ratio, roughness)
		{}

		glm::vec3 Metal::sampleWi(const glm::vec3& wo_tangent, core::UniformSampler& sampler) const
		{
			return m_microfacet.sampleWi(wo_tangent, sampler);
		}

		glm::vec3 Metal::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const
		{
			return m_microfacet.getBsdf(wi_tangent, wo_tangent);
		}

		float Metal::getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const
		{
			return m_microfacet.getPdf(wi_tangent, wo_tangent);
		}

		bool Metal::hasDeltaDistribution() const
		{
			return false;
		}

		bool Metal::useMultipleImportanceSampling() const
		{
			return true;
		}
	}
}