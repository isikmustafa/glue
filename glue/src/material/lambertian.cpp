#include "lambertian.h"

#include <glm\gtc\constants.hpp>
#include <glm\geometric.hpp>
#include <glm\trigonometric.hpp>

namespace glue
{
	namespace material
	{
		Lambertian::Lambertian(const glm::vec3& kd)
			: m_kd(kd * glm::one_over_pi<float>())
		{}

		geometry::SphericalCoordinate Lambertian::sampleDirection(core::UniformSampler& sampler) const
		{
			//Sample from cosine-weighted distribution.
			return geometry::SphericalCoordinate(1.0f, glm::acos(glm::sqrt(sampler.sample())), glm::two_pi<float>() * sampler.sample());
		}

		glm::vec3 Lambertian::getBsdf(const geometry::SphericalCoordinate& wi, const geometry::SphericalCoordinate& wo) const
		{
			return m_kd;
		}

		float Lambertian::getPdf(const geometry::SphericalCoordinate& wi, const geometry::SphericalCoordinate& wo) const
		{
			//Cosine-weighted pdf.
			return glm::cos(wi.theta) * glm::one_over_pi<float>();
		}

		bool Lambertian::hasDeltaDistribution() const
		{
			return false;
		}

		bool Lambertian::useMultipleImportanceSampling() const
		{
			return false;
		}
	}
}