#ifndef __GLUE__MICROFACET__MICROFACETSCATTERING__
#define __GLUE__MICROFACET__MICROFACETSCATTERING__

namespace glue
{
	namespace microfacet
	{
		template<typename MicrofacetDistribution, bool tSampleVisibleNormals = true>
		class MicrofacetScattering
		{
		public:
			explicit MicrofacetScattering(float roughness);

			std::pair<glm::vec3, glm::vec3> sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler, float no_over_ni) const;
			glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, float no_over_ni) const;
			float getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, float no_over_ni) const;

		private:
			float m_a;
		};
	}
}

#include "microfacet_scattering.inl"

#endif