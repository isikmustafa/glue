#include "..\material\bsdf_material.h"

#include <glm\geometric.hpp>
#include <glm\trigonometric.hpp>

namespace glue
{
	namespace microfacet
	{
		template<typename Fresnel, typename MicrofacetDistribution>
		MicrofacetReflection<Fresnel, MicrofacetDistribution>::MicrofacetReflection(float roughness)
			: m_a(roughness)
		{}

		template<typename Fresnel, typename MicrofacetDistribution>
		std::pair<glm::vec3, glm::vec3> MicrofacetReflection<Fresnel, MicrofacetDistribution>::sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler,
			float no_over_ni) const
		{
			MicrofacetDistribution microfacet(m_a);

			auto wh = microfacet.sampleWh(sampler);
			auto wo = glm::reflect(-wi_tangent, wh);
			auto wi_wh = glm::dot(wi_tangent, wh);
			auto fg = Fresnel()(no_over_ni, glm::abs(wi_wh)) * microfacet.g1(wi_tangent, wh) * microfacet.g1(wo, wh);
			auto f = fg * glm::abs(wi_wh / (material::cosTheta(wi_tangent) * material::cosTheta(wh)));

			return std::make_pair(wo, glm::vec3(f));
		}

		template<typename Fresnel, typename MicrofacetDistribution>
		std::pair<glm::vec3, glm::vec3> MicrofacetReflection<Fresnel, MicrofacetDistribution>::sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler,
			const glm::vec3& no_over_ni, const glm::vec3& ko_over_ki) const
		{
			MicrofacetDistribution microfacet(m_a);

			auto wh = microfacet.sampleWh(sampler);
			auto wo = glm::reflect(-wi_tangent, wh);
			auto wi_wh = glm::dot(wi_tangent, wh);
			auto fg = Fresnel()(no_over_ni, ko_over_ki, glm::abs(wi_wh)) * microfacet.g1(wi_tangent, wh) * microfacet.g1(wo, wh);
			auto f = fg * glm::abs(wi_wh / (material::cosTheta(wi_tangent) * material::cosTheta(wh)));

			return std::make_pair(wo, f);
		}

		template<typename Fresnel, typename MicrofacetDistribution>
		glm::vec3 MicrofacetReflection<Fresnel, MicrofacetDistribution>::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, float no_over_ni) const
		{
			MicrofacetDistribution microfacet(m_a);

			auto wh = glm::normalize(wi_tangent + wo_tangent);
			auto f = Fresnel()(no_over_ni, glm::abs(glm::dot(wi_tangent, wh)));
			auto dg = microfacet.d(wh) * microfacet.g1(wi_tangent, wh) * microfacet.g1(wo_tangent, wh);

			auto brdf = f * dg / glm::abs(4.0f * material::cosTheta(wi_tangent) * material::cosTheta(wo_tangent));

			return glm::vec3(brdf);
		}

		template<typename Fresnel, typename MicrofacetDistribution>
		glm::vec3 MicrofacetReflection<Fresnel, MicrofacetDistribution>::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent,
			const glm::vec3& no_over_ni, const glm::vec3& ko_over_ki) const
		{
			MicrofacetDistribution microfacet(m_a);

			auto wh = glm::normalize(wi_tangent + wo_tangent);
			auto f = Fresnel()(no_over_ni, ko_over_ki, glm::abs(glm::dot(wi_tangent, wh)));
			auto dg = microfacet.d(wh) * microfacet.g1(wi_tangent, wh) * microfacet.g1(wo_tangent, wh);

			auto brdf = f * dg / glm::abs(4.0f * material::cosTheta(wi_tangent) * material::cosTheta(wo_tangent));

			return glm::vec3(brdf);
		}

		template<typename Fresnel, typename MicrofacetDistribution>
		float MicrofacetReflection<Fresnel, MicrofacetDistribution>::getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const
		{
			MicrofacetDistribution microfacet(m_a);

			auto wh = glm::normalize(wi_tangent + wo_tangent);
			return glm::abs(microfacet.d(wh) * material::cosTheta(wh) / (4.0f * glm::dot(wo_tangent, wh)));
		}
	}
}