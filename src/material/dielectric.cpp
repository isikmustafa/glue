#include "dielectric.h"
#include "../xml/node.h"
#include "../microfacet/microfacet_reflection.h"

namespace glue
{
	namespace material
	{
		Dielectric::Xml::Xml(const xml::Node& node)
		{
			node.parseChildText("IorN", &ior_n);
			roughness = texture::Texture::Xml::factory(node.child("Roughness", true));
		}

		Dielectric::Xml::Xml(float p_ior_n, std::unique_ptr<texture::Texture::Xml> p_roughness)
			: ior_n(p_ior_n)
			, roughness(std::move(p_roughness))
		{}

		std::unique_ptr<BsdfMaterial> Dielectric::Xml::create() const
		{
			return std::make_unique<Dielectric>(*this);
		}

		Dielectric::Dielectric(const Dielectric::Xml& xml)
			: m_ior_n(xml.ior_n)
			, m_roughness(xml.roughness->create())
		{}

        std::pair<int, float> Dielectric::chooseBsdf(const glm::vec3& wo_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const
		{
            return sampler.sample() < 0.5f ? std::make_pair(0, 0.5f) : std::make_pair(1, 0.5f);
		}

		std::pair<glm::vec3, glm::vec3> Dielectric::sampleWi(const glm::vec3& wo_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const
		{
			auto no_over_ni = core::math::cosTheta(wo_tangent) > 0.0f ? 1.0f / m_ior_n : m_ior_n;

			std::pair<glm::vec3, glm::vec3> wi_f;
			switch (intersection.bsdf_choice)
			{
				case 0:
				{
					microfacet::MicrofacetReflection<microfacet::GGXDistribution> microfacet(m_roughness->fetch(intersection).r);
					wi_f = microfacet.sampleWi(wo_tangent, sampler, 1.0f / no_over_ni);
				}
				break;

				case 1:
				{
					microfacet::MicrofacetRefraction<microfacet::GGXDistribution> microfacet(m_roughness->fetch(intersection).r);
					wi_f = microfacet.sampleWi(wo_tangent, sampler, no_over_ni);

					if (!intersection.radiance_transport)
					{
                        wi_f.second /= no_over_ni * no_over_ni;
					}
				}
				break;
			}

			return wi_f;
		}

		glm::vec3 Dielectric::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const
		{
			auto no_over_ni = core::math::cosTheta(wo_tangent) > 0.0f ? 1.0f / m_ior_n : m_ior_n;

			glm::vec3 bsdf;
			switch (intersection.bsdf_choice)
			{
				case 0:
				{
					microfacet::MicrofacetReflection<microfacet::GGXDistribution> microfacet(m_roughness->fetch(intersection).r);
					bsdf = microfacet.getBsdf(wi_tangent, wo_tangent, 1.0f / no_over_ni);
				}
				break;

				case 1:
				{
					microfacet::MicrofacetRefraction<microfacet::GGXDistribution> microfacet(m_roughness->fetch(intersection).r);
					bsdf = microfacet.getBsdf(wi_tangent, wo_tangent, no_over_ni);

					if (!intersection.radiance_transport)
					{
						bsdf /= no_over_ni * no_over_ni;
					}
				}
				break;
			}

			return bsdf;
		}

		float Dielectric::getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const
		{
			auto no_over_ni = core::math::cosTheta(wo_tangent) > 0.0f ? 1.0f / m_ior_n : m_ior_n;

			float pdf;
			switch (intersection.bsdf_choice)
			{
				case 0:
				{
					microfacet::MicrofacetReflection<microfacet::GGXDistribution> microfacet(m_roughness->fetch(intersection).r);
					pdf = microfacet.getPdf(wi_tangent, wo_tangent);
				}
				break;

				case 1:
				{
					microfacet::MicrofacetRefraction<microfacet::GGXDistribution> microfacet(m_roughness->fetch(intersection).r);
					pdf = microfacet.getPdf(wi_tangent, wo_tangent, no_over_ni);
				}
				break;
			}

			return pdf;
		}

		bool Dielectric::hasDeltaDistribution(const geometry::Intersection& intersection) const
		{
			return false;
		}

		bool Dielectric::useMultipleImportanceSampling(const geometry::Intersection& intersection) const
		{
			return true;
		}

		bool Dielectric::isSpecular(const geometry::Intersection& intersection) const
		{
			return true;
		}
	}
}