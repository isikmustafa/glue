#include "..\material\bsdf_material.h"

#include <glm\geometric.hpp>
#include <glm\trigonometric.hpp>

namespace glue
{
	namespace microfacet
	{
		template<typename FresnelCallable, typename MicrofacetDistribution>
		MicrofacetReflection<FresnelCallable, MicrofacetDistribution>::MicrofacetReflection(const glm::vec3& n_ratio, const glm::vec3& k_ratio, float roughness)
			: m_microfacet(roughness)
			, m_n_ratio(n_ratio)
			, m_k_ratio(k_ratio)
		{}

		template<typename FresnelCallable, typename MicrofacetDistribution>
		glm::vec3 MicrofacetReflection<FresnelCallable, MicrofacetDistribution>::sampleWi(const glm::vec3& wo_tangent, core::UniformSampler& sampler) const
		{
			auto wh = m_microfacet.sampleWh(sampler);
			return 2.0f * glm::dot(wo_tangent, wh) * wh - wo_tangent;
		}

		template<typename FresnelCallable, typename MicrofacetDistribution>
		glm::vec3 MicrofacetReflection<FresnelCallable, MicrofacetDistribution>::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const
		{
			auto wh = glm::normalize(wi_tangent + wo_tangent);
			auto f = FresnelCallable()(m_n_ratio, m_k_ratio, glm::abs(glm::dot(wo_tangent, wh)));
			auto dg = m_microfacet.d(wh) * m_microfacet.g1(wi_tangent, wh) * m_microfacet.g1(wo_tangent, wh);

			auto brdf = f * dg / (4.0f * glm::abs(material::cosTheta(wi_tangent)) * glm::abs(material::cosTheta(wo_tangent)));

			return glm::vec3(brdf);
		}

		template<typename FresnelCallable, typename MicrofacetDistribution>
		float MicrofacetReflection<FresnelCallable, MicrofacetDistribution>::getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const
		{
			auto wh = glm::normalize(wi_tangent + wo_tangent);
			return m_microfacet.d(wh) * glm::abs(material::cosTheta(wh)) / (4.0f * glm::dot(wo_tangent, wh));
		}
	}
}