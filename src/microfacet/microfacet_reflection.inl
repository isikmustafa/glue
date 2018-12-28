#include "../core/math.h"
#include "../microfacet/fresnel.h"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

namespace glue
{
	namespace microfacet
	{
		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		MicrofacetReflection<MicrofacetDistribution, tSampleVisibleNormals>::MicrofacetReflection(float roughness)
			: m_microfacet(roughness)
		{}

		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		std::pair<glm::vec3, glm::vec3> MicrofacetReflection<MicrofacetDistribution, tSampleVisibleNormals>::sampleWi(const glm::vec3& wo_tangent, core::UniformSampler& sampler,
				float nt_over_ni) const
		{
			glm::vec3 wh;
			if constexpr (tSampleVisibleNormals)
			{
				wh = m_microfacet.sampleWhHd14(wo_tangent, sampler);
			}
			else
			{
				wh = m_microfacet.sampleWhWmlt07(sampler);
			}
			auto wo_wh = glm::dot(wo_tangent, wh);

			auto wi_tangent = glm::reflect(-wo_tangent, wh);
			float f;
			auto fresnel = fresnel::Dielectric()(nt_over_ni, glm::abs(wo_wh));
			if constexpr (tSampleVisibleNormals)
			{
				f = fresnel * m_microfacet.g1(wi_tangent, wh);
			}
			else
			{
				auto fg = fresnel * m_microfacet.g1(wi_tangent, wh) * m_microfacet.g1(wo_tangent, wh);
				f = fg * glm::abs(wo_wh / (core::math::cosTheta(wo_tangent) * core::math::cosTheta(wh)));
			}

			return std::make_pair(wi_tangent, glm::vec3(f));
		}

		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		std::pair<glm::vec3, glm::vec3> MicrofacetReflection<MicrofacetDistribution, tSampleVisibleNormals>::sampleWi(const glm::vec3& wo_tangent, core::UniformSampler& sampler,
			const glm::vec3& nt_over_ni, const glm::vec3& kt_over_ki) const
		{
			glm::vec3 wh;
			if constexpr (tSampleVisibleNormals)
			{
				wh = m_microfacet.sampleWhHd14(wo_tangent, sampler);
			}
			else
			{
				wh = m_microfacet.sampleWhWmlt07(sampler);
			}
			auto wo_wh = glm::dot(wo_tangent, wh);

			auto wi_tangent = glm::reflect(-wo_tangent, wh);
			glm::vec3 f;
			auto fresnel = fresnel::Conductor()(nt_over_ni, kt_over_ki, glm::abs(wo_wh));
			if constexpr (tSampleVisibleNormals)
			{
				f = fresnel * m_microfacet.g1(wi_tangent, wh);
			}
			else
			{
				auto fg = fresnel * m_microfacet.g1(wi_tangent, wh) * m_microfacet.g1(wo_tangent, wh);
				f = fg * glm::abs(wo_wh / (core::math::cosTheta(wo_tangent) * core::math::cosTheta(wh)));
			}

			return std::make_pair(wi_tangent, f);
		}

		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		glm::vec3 MicrofacetReflection<MicrofacetDistribution, tSampleVisibleNormals>::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, float nt_over_ni) const
		{
			auto wh = glm::normalize(wi_tangent + wo_tangent);
			auto fresnel = fresnel::Dielectric()(nt_over_ni, glm::abs(glm::dot(wi_tangent, wh)));
			auto dg = m_microfacet.d(wh) * m_microfacet.g1(wi_tangent, wh) * m_microfacet.g1(wo_tangent, wh);

			auto brdf = fresnel * dg / glm::abs(4.0f * core::math::cosTheta(wi_tangent) * core::math::cosTheta(wo_tangent));

			return glm::vec3(brdf);
		}

		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		glm::vec3 MicrofacetReflection<MicrofacetDistribution, tSampleVisibleNormals>::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent,
			const glm::vec3& nt_over_ni, const glm::vec3& kt_over_ki) const
		{
			auto wh = glm::normalize(wi_tangent + wo_tangent);
			auto fresnel = fresnel::Conductor()(nt_over_ni, kt_over_ki, glm::abs(glm::dot(wi_tangent, wh)));
			auto dg = m_microfacet.d(wh) * m_microfacet.g1(wi_tangent, wh) * m_microfacet.g1(wo_tangent, wh);

			auto brdf = fresnel * dg / glm::abs(4.0f * core::math::cosTheta(wi_tangent) * core::math::cosTheta(wo_tangent));

			return brdf;
		}

		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		float MicrofacetReflection<MicrofacetDistribution, tSampleVisibleNormals>::getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const
		{
			auto wh = glm::normalize(wi_tangent + wo_tangent);
			if constexpr (tSampleVisibleNormals)
			{
				return glm::abs(m_microfacet.pdfHd14(wo_tangent, wh) / (4.0f * glm::dot(wo_tangent, wh)));
			}
			else
			{
				return glm::abs(m_microfacet.pdfWmlt07(wh) / (4.0f * glm::dot(wo_tangent, wh)));
			}
		}
	}
}