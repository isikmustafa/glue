#include "metal.h"
#include "..\core\real_sampler.h"
#include "..\xml\node.h"

namespace glue
{
	namespace material
	{
		Metal::Xml::Xml(const xml::Node& node)
		{
			node.parseChildText("IorN", &ior_n.x, &ior_n.y, &ior_n.z);
			node.parseChildText("IorK", &ior_k.x, &ior_k.y, &ior_k.z);
			node.parseChildText("Roughness", &roughness);
		}

		Metal::Xml::Xml(const glm::vec3& p_ior_n, const glm::vec3& p_ior_k, float p_roughness)
			: ior_n(p_ior_n)
			, ior_k(p_ior_k)
			, roughness(p_roughness)
		{}

		std::unique_ptr<BsdfMaterial> Metal::Xml::create() const
		{
			return std::make_unique<Metal>(*this);
		}

		Metal::Metal(const Metal::Xml& xml)
			: m_ior_n(xml.ior_n)
			, m_ior_k(xml.ior_k)
			, m_microfacet(xml.roughness)
			, m_use_mis(xml.roughness < 0.1f)
		{}

		std::pair<glm::vec3, glm::vec3> Metal::sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler) const
		{
			return m_microfacet.sampleWo(wi_tangent, sampler, m_ior_n, m_ior_k);
		}

		glm::vec3 Metal::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const
		{
			return m_microfacet.getBsdf(wi_tangent, wo_tangent, m_ior_n, m_ior_k);
		}

		float Metal::getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const
		{
			return m_microfacet.getPdf(wi_tangent, wo_tangent);
		}

		bool Metal::hasDeltaDistribution() const
		{
			return false;
		}

		bool Metal::useMultipleImportanceSampling() const
		{
			return m_use_mis;
		}
	}
}