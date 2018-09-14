#include "dielectric.h"
#include "..\core\real_sampler.h"
#include "..\core\math.h"
#include "..\xml\node.h"

namespace glue
{
	namespace material
	{
		Dielectric::Xml::Xml(const xml::Node& node)
		{
			node.parseChildText("IorN", &ior_n);
			node.parseChildText("Roughness", &roughness);
		}

		Dielectric::Xml::Xml(float p_ior_n, float p_roughness)
			: ior_n(p_ior_n)
			, roughness(p_roughness)
		{}

		std::unique_ptr<BsdfMaterial> Dielectric::Xml::create() const
		{
			return std::make_unique<Dielectric>(*this);
		}

		Dielectric::Dielectric(const Dielectric::Xml& xml)
			: m_ior_n(xml.ior_n)
			, m_microfacet(xml.roughness)
		{}

		std::pair<glm::vec3, glm::vec3> Dielectric::sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const
		{
			return m_microfacet.sampleWo(wi_tangent, sampler, core::math::cosTheta(wi_tangent) > 0.0f ? m_ior_n : 1.0f / m_ior_n);
		}

		glm::vec3 Dielectric::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const
		{
			return m_microfacet.getBsdf(wi_tangent, wo_tangent, core::math::cosTheta(wi_tangent) > 0.0f ? m_ior_n : 1.0f / m_ior_n);
		}

		float Dielectric::getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const
		{
			return m_microfacet.getPdf(wi_tangent, wo_tangent, core::math::cosTheta(wi_tangent) > 0.0f ? m_ior_n : 1.0f / m_ior_n);
		}

		bool Dielectric::hasDeltaDistribution() const
		{
			return false;
		}

		bool Dielectric::useMultipleImportanceSampling() const
		{
			return true;
		}
	}
}