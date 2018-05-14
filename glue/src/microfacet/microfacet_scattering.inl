#include "..\material\bsdf_material.h"

#include <glm\geometric.hpp>
#include <glm\trigonometric.hpp>

namespace glue
{
	namespace microfacet
	{
		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		MicrofacetScattering<MicrofacetDistribution, tSampleVisibleNormals>::MicrofacetScattering(float roughness)
			: m_a(roughness)
		{}

		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		std::pair<glm::vec3, glm::vec3> MicrofacetScattering<MicrofacetDistribution, tSampleVisibleNormals>::sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler, float no_over_ni) const
		{
			MicrofacetDistribution microfacet(m_a);
			if constexpr (!tSampleVisibleNormals)
			{
				//Walter's trick to reduce sampling weight.
				auto alpha = (1.2f - 0.2f * glm::sqrt(glm::abs(material::cosTheta(wi_tangent)))) * m_a;
				microfacet = MicrofacetDistribution(alpha);
			}

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

			glm::vec3 wo_tangent;

			//Reflection
			if (sampler.sample() < fresnel::Dielectric()(no_over_ni, glm::abs(wi_wh)))
			{
				wo_tangent = glm::reflect(-wi_tangent, wh);
			}
			//Transmission
			else
			{
				wo_tangent = glm::refract(-wi_tangent, material::cosTheta(wi_tangent) > 0.0f ? wh : -wh, 1.0f / no_over_ni);
			}

			float f;
			if constexpr (tSampleVisibleNormals)
			{
				f = microfacet.g1(wo_tangent, wh);
			}
			else
			{
				f = microfacet.g1(wi_tangent, wh) * microfacet.g1(wo_tangent, wh) * glm::abs(wi_wh / (material::cosTheta(wi_tangent) * material::cosTheta(wh)));
			}

			return std::make_pair(wo_tangent, glm::vec3(f));
		}

		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		glm::vec3 MicrofacetScattering<MicrofacetDistribution, tSampleVisibleNormals>::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, float no_over_ni) const
		{
			MicrofacetDistribution microfacet(m_a);
			if constexpr (!tSampleVisibleNormals)
			{
				//Walter's trick to reduce sampling weight.
				auto alpha = (1.2f - 0.2f * glm::sqrt(glm::abs(material::cosTheta(wi_tangent)))) * m_a;
				microfacet = MicrofacetDistribution(alpha);
			}

			glm::vec3 bsdf;

			//Reflection
			if (material::cosTheta(wi_tangent) * material::cosTheta(wo_tangent) > 0.0f)
			{
				auto wh = glm::normalize(wi_tangent + wo_tangent);
				auto wi_wh = glm::dot(wi_tangent, wh);
				auto fresnel = fresnel::Dielectric()(no_over_ni, glm::abs(wi_wh));
				auto dg = microfacet.d(wh) * microfacet.g1(wi_tangent, wh) * microfacet.g1(wo_tangent, wh);

				auto brdf = fresnel * dg / glm::abs(4.0f * material::cosTheta(wi_tangent) * material::cosTheta(wo_tangent));

				bsdf = glm::vec3(brdf);
			}
			//Transmission
			else
			{
				auto wh = -glm::normalize(wi_tangent + no_over_ni * wo_tangent);
				auto wi_wh = glm::dot(wi_tangent, wh);
				auto wo_wh = glm::dot(wo_tangent, wh);

				auto x = glm::abs((wi_wh * wo_wh) / (material::cosTheta(wi_tangent) * material::cosTheta(wo_tangent)));
				auto fresnel = fresnel::Dielectric()(no_over_ni, glm::abs(wi_wh));
				auto dg = microfacet.d(wh) * microfacet.g1(wi_tangent, wh) * microfacet.g1(wo_tangent, wh);
				auto denom = wi_wh + no_over_ni * wo_wh;

				auto btdf = x * (1.0f - fresnel) * dg * no_over_ni * no_over_ni / (denom * denom);

				bsdf = glm::vec3(btdf);
			}

			return bsdf;
		}

		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		float MicrofacetScattering<MicrofacetDistribution, tSampleVisibleNormals>::getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, float no_over_ni) const
		{
			MicrofacetDistribution microfacet(m_a);
			if constexpr (!tSampleVisibleNormals)
			{
				//Walter's trick to reduce sampling weight.
				auto alpha = (1.2f - 0.2f * glm::sqrt(glm::abs(material::cosTheta(wi_tangent)))) * m_a;
				microfacet = MicrofacetDistribution(alpha);
			}

			float pdf;

			//Reflection
			if (material::cosTheta(wi_tangent) * material::cosTheta(wo_tangent) > 0.0f)
			{
				auto wh = glm::normalize(wi_tangent + wo_tangent);
				auto wi_wh = glm::dot(wi_tangent, wh);

				auto fresnel = fresnel::Dielectric()(no_over_ni, glm::abs(wi_wh));

				if constexpr (tSampleVisibleNormals)
				{
					pdf = fresnel * glm::abs(microfacet.pdfHd14(wi_tangent, wh) / (4.0f * glm::dot(wo_tangent, wh)));
				}
				else
				{
					pdf = fresnel * glm::abs(microfacet.pdfWmlt07(wh) / (4.0f * glm::dot(wo_tangent, wh)));
				}
			}
			//Transmission
			else
			{
				auto wh = -glm::normalize(wi_tangent + no_over_ni * wo_tangent);
				auto wi_wh = glm::dot(wi_tangent, wh);
				auto wo_wh = glm::dot(wo_tangent, wh);

				auto fresnel = fresnel::Dielectric()(no_over_ni, glm::abs(wi_wh));
				auto denom = wi_wh + no_over_ni * wo_wh;

				if constexpr (tSampleVisibleNormals)
				{
					pdf = (1.0f - fresnel) * glm::abs(microfacet.pdfHd14(wi_tangent, wh) * wo_wh * no_over_ni * no_over_ni / (denom * denom));
				}
				else
				{
					pdf = (1.0f - fresnel) * glm::abs(microfacet.pdfWmlt07(wh) * wo_wh * no_over_ni * no_over_ni / (denom * denom));
				}
			}

			return pdf;
		}
	}
}