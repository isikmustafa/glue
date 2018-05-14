#include "..\material\bsdf_material.h"

#include <glm\geometric.hpp>
#include <glm\trigonometric.hpp>

namespace glue
{
	namespace microfacet
	{
		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		MicrofacetReflection<MicrofacetDistribution, tSampleVisibleNormals>::MicrofacetReflection(float roughness)
			: m_a(roughness)
		{}

		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		std::pair<glm::vec3, glm::vec3> MicrofacetReflection<MicrofacetDistribution, tSampleVisibleNormals>::sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler,
			float no_over_ni) const
		{
			MicrofacetDistribution microfacet(m_a);

			glm::vec3 wh;
			if constexpr (tSampleVisibleNormals)
			{
				wh = microfacet.sampleWhHd14(wi_tangent, sampler);
			}
			else
			{
				wh = microfacet.sampleWhWmlt07(sampler);
			}
			auto wi_wh = glm::dot(wi_tangent, wh);

			auto wo_tangent = glm::reflect(-wi_tangent, wh);
			float f;
			auto fresnel = fresnel::Dielectric()(no_over_ni, glm::abs(wi_wh));
			if constexpr (tSampleVisibleNormals)
			{
				f = fresnel * microfacet.g1(wo_tangent, wh);
			}
			else
			{
				auto fg = fresnel * microfacet.g1(wi_tangent, wh) * microfacet.g1(wo_tangent, wh);
				f = fg * glm::abs(wi_wh / (material::cosTheta(wi_tangent) * material::cosTheta(wh)));
			}

			return std::make_pair(wo_tangent, glm::vec3(f));
		}

		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		std::pair<glm::vec3, glm::vec3> MicrofacetReflection<MicrofacetDistribution, tSampleVisibleNormals>::sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler,
			const glm::vec3& no_over_ni, const glm::vec3& ko_over_ki) const
		{
			MicrofacetDistribution microfacet(m_a);

			glm::vec3 wh;
			if constexpr (tSampleVisibleNormals)
			{
				wh = microfacet.sampleWhHd14(wi_tangent, sampler);
			}
			else
			{
				wh = microfacet.sampleWhWmlt07(sampler);
			}
			auto wi_wh = glm::dot(wi_tangent, wh);

			auto wo_tangent = glm::reflect(-wi_tangent, wh);
			glm::vec3 f;
			auto fresnel = fresnel::Conductor()(no_over_ni, ko_over_ki, glm::abs(wi_wh));
			if constexpr (tSampleVisibleNormals)
			{
				f = fresnel * microfacet.g1(wo_tangent, wh);
			}
			else
			{
				auto fg = fresnel * microfacet.g1(wi_tangent, wh) * microfacet.g1(wo_tangent, wh);
				f = fg * glm::abs(wi_wh / (material::cosTheta(wi_tangent) * material::cosTheta(wh)));
			}

			return std::make_pair(wo_tangent, f);
		}

		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		glm::vec3 MicrofacetReflection<MicrofacetDistribution, tSampleVisibleNormals>::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, float no_over_ni) const
		{
			MicrofacetDistribution microfacet(m_a);

			auto wh = glm::normalize(wi_tangent + wo_tangent);
			auto fresnel = fresnel::Dielectric()(no_over_ni, glm::abs(glm::dot(wi_tangent, wh)));
			auto dg = microfacet.d(wh) * microfacet.g1(wi_tangent, wh) * microfacet.g1(wo_tangent, wh);

			auto brdf = fresnel * dg / glm::abs(4.0f * material::cosTheta(wi_tangent) * material::cosTheta(wo_tangent));

			return glm::vec3(brdf);
		}

		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		glm::vec3 MicrofacetReflection<MicrofacetDistribution, tSampleVisibleNormals>::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent,
			const glm::vec3& no_over_ni, const glm::vec3& ko_over_ki) const
		{
			MicrofacetDistribution microfacet(m_a);

			auto wh = glm::normalize(wi_tangent + wo_tangent);
			auto fresnel = fresnel::Conductor()(no_over_ni, ko_over_ki, glm::abs(glm::dot(wi_tangent, wh)));
			auto dg = microfacet.d(wh) * microfacet.g1(wi_tangent, wh) * microfacet.g1(wo_tangent, wh);

			auto brdf = fresnel * dg / glm::abs(4.0f * material::cosTheta(wi_tangent) * material::cosTheta(wo_tangent));

			return brdf;
		}

		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		float MicrofacetReflection<MicrofacetDistribution, tSampleVisibleNormals>::getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const
		{
			MicrofacetDistribution microfacet(m_a);

			auto wh = glm::normalize(wi_tangent + wo_tangent);
			if constexpr (tSampleVisibleNormals)
			{
				return glm::abs(microfacet.pdfHd14(wi_tangent, wh) / (4.0f * glm::dot(wo_tangent, wh)));
			}
			else
			{
				return glm::abs(microfacet.pdfWmlt07(wh) / (4.0f * glm::dot(wo_tangent, wh)));
			}
		}
	}
}