#include "metal.h"
#include "..\core\uniform_sampler.h"

namespace glue
{
	namespace material
	{
		Metal::Metal(const glm::vec3& ior_n, const glm::vec3& ior_k, float roughness)
			: m_ior_n(ior_n)
			, m_ior_k(ior_k)
			, m_microfacet(roughness)
		{}

		std::pair<glm::vec3, glm::vec3> Metal::sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler) const
		{
			return m_microfacet.sampleWo(wi_tangent, sampler, m_ior_n, m_ior_k);
		}

		glm::vec3 Metal::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const
		{
			return m_microfacet.getBsdf(wi_tangent, wo_tangent, m_ior_n, m_ior_k);
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