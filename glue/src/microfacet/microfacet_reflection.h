#ifndef __GLUE__MICROFACET__MICROFACETREFLECTION__
#define __GLUE__MICROFACET__MICROFACETREFLECTION__

namespace glue
{
	namespace microfacet
	{
		template<typename Fresnel, typename MicrofacetDistribution>
		class MicrofacetReflection
		{
		public:
			MicrofacetReflection(float roughness);

			std::pair<glm::vec3, glm::vec3> sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler, float no_over_ni) const;
			std::pair<glm::vec3, glm::vec3> sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler, const glm::vec3& no_over_ni, const glm::vec3& ko_over_ki) const;
			glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, float no_over_ni) const;
			glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const glm::vec3& no_over_ni, const glm::vec3& ko_over_ki) const;
			float getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const;

		private:
			float m_a;
		};
	}
}

#include "microfacet_reflection.inl"

#endif