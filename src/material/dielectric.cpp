#include "dielectric.h"
#include "../xml/node.h"

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

		std::pair<glm::vec3, glm::vec3> Dielectric::sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const
		{
			microfacet::MicrofacetScattering<microfacet::GGXDistribution> microfacet(m_roughness->fetch(intersection).r);
			return microfacet.sampleWo(wi_tangent, sampler, core::math::cosTheta(wi_tangent) > 0.0f ? m_ior_n : 1.0f / m_ior_n);
		}

		glm::vec3 Dielectric::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const
		{
			microfacet::MicrofacetScattering<microfacet::GGXDistribution> microfacet(m_roughness->fetch(intersection).r);
			return microfacet.getBsdf(wi_tangent, wo_tangent, core::math::cosTheta(wi_tangent) > 0.0f ? m_ior_n : 1.0f / m_ior_n);
		}

		float Dielectric::getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const
		{
			microfacet::MicrofacetScattering<microfacet::GGXDistribution> microfacet(m_roughness->fetch(intersection).r);
			return microfacet.getPdf(wi_tangent, wo_tangent, core::math::cosTheta(wi_tangent) > 0.0f ? m_ior_n : 1.0f / m_ior_n);
		}

		bool Dielectric::hasDeltaDistribution(const geometry::Intersection& intersection) const
		{
			return false;
		}

		bool Dielectric::useMultipleImportanceSampling(const geometry::Intersection& intersection) const
		{
			return true;
		}
	}
}