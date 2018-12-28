#ifndef __GLUE__MICROFACET__MICROFACETREFRACTION__
#define __GLUE__MICROFACET__MICROFACETREFRACTION__

namespace glue
{
	namespace microfacet
	{
		template<typename MicrofacetDistribution, bool tSampleVisibleNormals = true>
		class MicrofacetRefraction
		{
		public:
			explicit MicrofacetRefraction(float roughness);

			std::pair<glm::vec3, glm::vec3> sampleWi(const glm::vec3& wo_tangent, core::UniformSampler& sampler, float nt_over_ni) const;
			glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, float nt_over_ni) const;
			float getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, float nt_over_ni) const;

		private:
			float m_a;
		};
	}
}

#include "microfacet_refraction.inl"

#endif