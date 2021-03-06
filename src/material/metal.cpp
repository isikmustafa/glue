#include "metal.h"
#include "../core/real_sampler.h"
#include "../xml/node.h"

namespace glue
{
	namespace material
	{
		Metal::Xml::Xml(const xml::Node& node)
		{
			node.parseChildText("IorN", &ior_n.x, &ior_n.y, &ior_n.z);
			node.parseChildText("IorK", &ior_k.x, &ior_k.y, &ior_k.z);
			roughness = texture::Texture::Xml::factory(node.child("Roughness", true));
		}

		Metal::Xml::Xml(const glm::vec3& p_ior_n, const glm::vec3& p_ior_k, std::unique_ptr<texture::Texture::Xml> p_roughness)
			: ior_n(p_ior_n)
			, ior_k(p_ior_k)
			, roughness(std::move(p_roughness))
		{}

		std::unique_ptr<BsdfMaterial> Metal::Xml::create() const
		{
			return std::make_unique<Metal>(*this);
		}

		Metal::Metal(const Metal::Xml& xml)
			: m_ior_n(xml.ior_n)
			, m_ior_k(xml.ior_k)
			, m_roughness(xml.roughness->create())
		{}

		std::pair<glm::vec3, glm::vec3> Metal::sampleWi(const glm::vec3& wo_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const
		{
			microfacet::MicrofacetReflection<microfacet::GGXDistribution> microfacet(m_roughness->fetch(intersection).r);
			return microfacet.sampleWi(wo_tangent, sampler, m_ior_n, m_ior_k);
		}

		glm::vec3 Metal::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const
		{
			microfacet::MicrofacetReflection<microfacet::GGXDistribution> microfacet(m_roughness->fetch(intersection).r);
			return microfacet.getBsdf(wi_tangent, wo_tangent, m_ior_n, m_ior_k);
		}

		float Metal::getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const
		{
			microfacet::MicrofacetReflection<microfacet::GGXDistribution> microfacet(m_roughness->fetch(intersection).r);
			return microfacet.getPdf(wi_tangent, wo_tangent);
		}

		bool Metal::hasDeltaDistribution(const geometry::Intersection& intersection) const
		{
			return false;
		}

		bool Metal::useMultipleImportanceSampling(const geometry::Intersection& intersection) const
		{
			return true;
		}

		bool Metal::isSpecular(const geometry::Intersection& intersection) const
		{
			return true;
		}
	}
}