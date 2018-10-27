#include "smooth_layered.h"
#include "..\geometry\spherical_coordinate.h"
#include "..\microfacet\fresnel.h"
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
		SmoothLayered::Xml::Xml(const xml::Node& node)
		{
			kd = texture::Texture::Xml::factory(node.child("Kd", true));
			node.parseChildText("IorN", &ior_n);
		}

		SmoothLayered::Xml::Xml(std::unique_ptr<texture::Texture::Xml> p_kd, float p_ior_n)
			: kd(std::move(p_kd))
			, ior_n(p_ior_n)
		{}

		std::unique_ptr<BsdfMaterial> SmoothLayered::Xml::create() const
		{
			return std::make_unique<SmoothLayered>(*this);
		}

		SmoothLayered::SmoothLayered(const SmoothLayered::Xml& xml)
			: m_kd(xml.kd->create())
			, m_ior_n(xml.ior_n)
			, m_fsum(0.0f)
			, m_sinthetac(glm::sin(microfacet::fresnel::Dielectric().criticAngle(1.0f / m_ior_n)))
		{
			core::UniformSampler sampler;
			constexpr int N = 100000;
			for (int i = 0; i < N; ++i)
			{
				auto dir = geometry::SphericalCoordinate(1.0f, glm::acos(glm::sqrt(sampler.sample())), glm::two_pi<float>() * sampler.sample()).toCartesianCoordinate();				m_fsum += microfacet::fresnel::Dielectric()(1.0f / m_ior_n, core::math::cosTheta(dir));
			}
			m_fsum /= N;
		}

		int SmoothLayered::chooseBsdf(const glm::vec3& wi_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const
		{
			auto fresnel = microfacet::fresnel::Dielectric()(m_ior_n, core::math::cosTheta(wi_tangent));

			return sampler.sample() < fresnel ? 0 : 1;
		}

		std::pair<glm::vec3, glm::vec3> SmoothLayered::sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const
		{
			glm::vec3 wo(0.0f);
			glm::vec3 f(0.0f);

			if (core::math::cosTheta(wi_tangent) > 0.0f)
			{
				switch (intersection.bsdf_choice)
				{
				case 0:
					wo = glm::reflect(-wi_tangent, glm::vec3(0.0f, 0.0f, 1.0f));
					f = glm::vec3(1.0f);
					break;

				case 1:
					//Multiplying by m_sinthetac restricts theta to be in [0, theta_critical].
					auto theta = glm::asin(glm::sqrt(sampler.sample()) * m_sinthetac);
					auto wo_inside = geometry::SphericalCoordinate(1.0f, theta, glm::two_pi<float>() * sampler.sample()).toCartesianCoordinate();

					wo = glm::refract(wo_inside, glm::vec3(0.0f, 0.0f, -1.0f), m_ior_n);
					auto f_in = microfacet::fresnel::Dielectric()(m_ior_n, core::math::cosTheta(wo));
					auto kd = m_kd->fetch(intersection);
					f = (1.0f - f_in) * kd * (m_sinthetac * m_sinthetac) / (1.0f - m_fsum * kd);
					break;
				}
			}

			return std::make_pair(wo, f);
		}

		glm::vec3 SmoothLayered::getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const
		{
			if (core::math::cosTheta(wi_tangent) > 0.0f && core::math::cosTheta(wo_tangent) > 0.0f)
			{
				auto f_out = microfacet::fresnel::Dielectric()(m_ior_n, core::math::cosTheta(wi_tangent));

				switch (intersection.bsdf_choice)
				{
				case 0:
					return glm::vec3(f_out) / core::math::cosTheta(wo_tangent);

				case 1:
					auto wo_inside = -glm::refract(-wo_tangent, glm::vec3(0.0f, 0.0f, 1.0f), 1.0f / m_ior_n);

					//Does not obey reciprocity.
					auto dwins_over_dwi = (1.0f / (m_ior_n * m_ior_n)) * glm::abs(glm::dot(wo_tangent, wo_inside));
					auto f_in = microfacet::fresnel::Dielectric()(m_ior_n, core::math::cosTheta(wo_tangent));

					auto kd = m_kd->fetch(intersection);
					auto nom = (1.0f - f_in) * (1.0f - f_out) * core::math::cosTheta(wo_inside) * kd;
					auto denom = glm::pi<float>() * (1.0f - m_fsum * kd) * core::math::cosTheta(wo_tangent);

					return dwins_over_dwi * nom / denom;
				}
			}

			return glm::vec3(0.0f);
		}

		float SmoothLayered::getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const
		{
			if (core::math::cosTheta(wi_tangent) > 0.0f && core::math::cosTheta(wo_tangent) > 0.0f)
			{
				if (intersection.bsdf_choice == 1)
				{
					auto wo_inside = -glm::refract(-wo_tangent, glm::vec3(0.0f, 0.0f, 1.0f), 1.0f / m_ior_n);
					auto fresnel = microfacet::fresnel::Dielectric()(m_ior_n, core::math::cosTheta(wi_tangent));
					auto dwins_over_dwi = (1.0f / (m_ior_n * m_ior_n)) * glm::abs(glm::dot(wo_tangent, wo_inside));

					return (1.0f - fresnel) * dwins_over_dwi * core::math::cosTheta(wo_inside) * glm::one_over_pi<float>() / (m_sinthetac * m_sinthetac);
				}
			}

			return 0.0f;
		}

		bool SmoothLayered::hasDeltaDistribution(const geometry::Intersection& intersection) const
		{
			return intersection.bsdf_choice == 0;
		}

		bool SmoothLayered::useMultipleImportanceSampling(const geometry::Intersection& intersection) const
		{
			return false;
		}
	}
}