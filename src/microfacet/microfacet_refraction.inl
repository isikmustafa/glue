#include "../core/math.h"
#include "../core/real_sampler.h"
#include "../microfacet/fresnel.h"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

namespace glue
{
	namespace microfacet
	{
		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		MicrofacetRefraction<MicrofacetDistribution, tSampleVisibleNormals>::MicrofacetRefraction(float roughness)
			: m_a(roughness)
		{}

		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		std::pair<glm::vec3, glm::vec3> MicrofacetRefraction<MicrofacetDistribution, tSampleVisibleNormals>::sampleWi(const glm::vec3& wo_tangent, core::UniformSampler& sampler,
		        float nt_over_ni) const
		{
			MicrofacetDistribution microfacet(m_a);
			if constexpr (!tSampleVisibleNormals)
			{
				//Walter's trick to reduce sampling weight.
				auto alpha = (1.2f - 0.2f * glm::sqrt(glm::abs(core::math::cosTheta(wo_tangent)))) * m_a;
				microfacet = MicrofacetDistribution(alpha);
			}

            glm::vec3 wh;
            if constexpr (tSampleVisibleNormals)
            {
                wh = microfacet.sampleWhHd14(wo_tangent, sampler);
            }
            else
            {
                wh = microfacet.sampleWhWmlt07(sampler);
            }
            auto wo_wh = glm::dot(wo_tangent, wh);

            auto wi_tangent = glm::refract(-wo_tangent, core::math::cosTheta(wo_tangent) > 0.0f ? wh : -wh, nt_over_ni);
			float f;
            auto fresnel = fresnel::Dielectric()(1.0f / nt_over_ni, glm::abs(wo_wh));
			if constexpr (tSampleVisibleNormals)
			{
                f = (1.0f - fresnel) * microfacet.g1(wi_tangent, wh);
			}
			else
			{
                auto fg = (1.0f - fresnel) * microfacet.g1(wi_tangent, wh) * microfacet.g1(wo_tangent, wh);
                f = fg * glm::abs(wo_wh / (core::math::cosTheta(wo_tangent) * core::math::cosTheta(wh)));
			}

			//Only for radiance transport.
			f *= nt_over_ni * nt_over_ni;

            return std::make_pair(wi_tangent, glm::vec3(f));
		}

		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		glm::vec3 MicrofacetRefraction<MicrofacetDistribution, tSampleVisibleNormals>::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, float nt_over_ni) const
		{
            MicrofacetDistribution microfacet(m_a);
            if constexpr (!tSampleVisibleNormals)
            {
                //Walter's trick to reduce sampling weight.
                auto alpha = (1.2f - 0.2f * glm::sqrt(glm::abs(core::math::cosTheta(wo_tangent)))) * m_a;
                microfacet = MicrofacetDistribution(alpha);
            }

            auto wh = -glm::normalize(wi_tangent + nt_over_ni * wo_tangent);
            auto wi_wh = glm::dot(wi_tangent, wh);
            auto wo_wh = glm::dot(wo_tangent, wh);

            auto x = glm::abs((wi_wh * wo_wh) / (core::math::cosTheta(wi_tangent) * core::math::cosTheta(wo_tangent)));
            auto fresnel = fresnel::Dielectric()(nt_over_ni, glm::abs(wi_wh));
            auto dg = microfacet.d(wh) * microfacet.g1(wi_tangent, wh) * microfacet.g1(wo_tangent, wh);
            auto denom = wi_wh + nt_over_ni * wo_wh;

            auto btdf = x * (1.0f - fresnel) * dg / (denom * denom);

            //Only for radiance transport.
            btdf *= nt_over_ni * nt_over_ni;

            return glm::vec3(btdf);
		}

		template<typename MicrofacetDistribution, bool tSampleVisibleNormals>
		float MicrofacetRefraction<MicrofacetDistribution, tSampleVisibleNormals>::getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, float nt_over_ni) const
		{
            MicrofacetDistribution microfacet(m_a);
            if constexpr (!tSampleVisibleNormals)
            {
                //Walter's trick to reduce sampling weight.
                auto alpha = (1.2f - 0.2f * glm::sqrt(glm::abs(core::math::cosTheta(wo_tangent)))) * m_a;
                microfacet = MicrofacetDistribution(alpha);
            }

            auto wh = -glm::normalize(wi_tangent + nt_over_ni * wo_tangent);
            auto wi_wh = glm::dot(wi_tangent, wh);
            auto wo_wh = glm::dot(wo_tangent, wh);

            auto fresnel = fresnel::Dielectric()(nt_over_ni, glm::abs(wi_wh));
            auto denom = wi_wh + nt_over_ni * wo_wh;

            if constexpr (tSampleVisibleNormals)
            {
                return glm::abs(microfacet.pdfHd14(wo_tangent, wh) * wi_wh / (denom * denom));
            }
            else
            {
                return glm::abs(microfacet.pdfWmlt07(wh) * wi_wh / (denom * denom));
            }
		}
	}
}