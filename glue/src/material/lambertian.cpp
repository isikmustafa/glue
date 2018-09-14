#include "lambertian.h"
#include "..\geometry\spherical_coordinate.h"
#include "..\core\real_sampler.h"
#include "..\core\math.h"
#include "..\xml\node.h"

#include <glm\gtc\constants.hpp>
#include <glm\geometric.hpp>
#include <glm\trigonometric.hpp>

namespace glue
{
	namespace material
	{
		Lambertian::Xml::Xml(const xml::Node& node)
		{
			kd = texture::Texture::Xml::factory(node.child("Kd", true));
		}

		Lambertian::Xml::Xml(std::unique_ptr<texture::Texture::Xml> p_kd)
			: kd(std::move(p_kd))
		{}

		std::unique_ptr<BsdfMaterial> Lambertian::Xml::create() const
		{
			return std::make_unique<Lambertian>(*this);
		}

		Lambertian::Lambertian(const Lambertian::Xml& xml)
			: m_kd(xml.kd->create())
		{}

		std::pair<glm::vec3, glm::vec3> Lambertian::sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const
		{
			//Sample from cosine-weighted distribution.
			auto wo = geometry::SphericalCoordinate(1.0f, glm::acos(glm::sqrt(sampler.sample())), glm::two_pi<float>() * sampler.sample()).toCartesianCoordinate();
			auto f = m_kd->fetch(intersection);

			return std::make_pair(wo, f);
		}

		glm::vec3 Lambertian::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const
		{
			return m_kd->fetch(intersection) * glm::one_over_pi<float>();
		}

		float Lambertian::getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const
		{
			//Cosine-weighted pdf.
			return glm::max(core::math::cosTheta(wo_tangent), 0.0f) * glm::one_over_pi<float>();
		}

		bool Lambertian::hasDeltaDistribution() const
		{
			return false;
		}

		bool Lambertian::useMultipleImportanceSampling() const
		{
			return true;
		}
	}
}