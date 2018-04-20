#ifndef __GLUE__MICROFACET__MICROFACETREFLECTION__
#define __GLUE__MICROFACET__MICROFACETREFLECTION__

namespace glue
{
	namespace microfacet
	{
		template<typename FresnelCallable, typename MicrofacetDistribution>
		class MicrofacetReflection
		{
		public:
			MicrofacetReflection(const glm::vec3& n_ratio, const glm::vec3& k_ratio, float roughness);

			glm::vec3 sampleWi(const glm::vec3& wo_tangent, core::UniformSampler& sampler) const;
			glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const;
			float getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const;

		private:
			MicrofacetDistribution m_microfacet;
			glm::vec3 m_n_ratio;
			glm::vec3 m_k_ratio;
		};
	}
}

#include "microfacet_reflection.inl"

#endif