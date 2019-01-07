#include "environment_light.h"
#include "../geometry/spherical_coordinate.h"
#include "../xml/node.h"
#include "../core/math.h"
#include "../core/scene.h"
#include "../geometry/mapper.h"

#include <glm/trigonometric.hpp>
#include <glm/gtc/constants.hpp>

namespace glue
{
	namespace light
	{
		EnvironmentLight::Xml::Xml(const xml::Node& node)
			: hdri(node.child("Texture", true))
			, transformation(node.child("Transformation") ? geometry::Transformation::Xml(node.child("Transformation")) : geometry::Transformation::Xml())
		{
			attributes = node.attributes();
		}

		std::unique_ptr<Light> EnvironmentLight::Xml::create() const
		{
			return std::make_unique<EnvironmentLight>(*this);
		}

		EnvironmentLight::EnvironmentLight(const EnvironmentLight::Xml& xml)
			: m_hdri(xml.hdri)
			, m_transformation(xml.transformation)
		{
			int width = m_hdri.getWidth();
			int height = m_hdri.getHeight();

			std::vector<std::vector<float>> pdfs(width, std::vector<float>(height));

			for (int i = 0; i < width; ++i)
			{
				for (int j = 0; j < height; ++j)
				{
					glm::vec2 uv((i + 0.5f) / width, (j + 0.5f) / height);
					pdfs[i][j] = core::math::rgbToLuminance(m_hdri.fetchTexelNearest(uv, 0)) * glm::sin(uv.y * glm::pi<float>());
				}
			}

			m_sampler = core::Discrete2DSampler(pdfs);
		}

		LightSample EnvironmentLight::sample(core::UniformSampler& sampler, const geometry::Intersection& intersection) const
		{
			LightSample light_sample;

			auto ij = m_sampler.sample(sampler);
			glm::vec2 uv((ij.first + 0.5f) / m_hdri.getWidth(), (ij.second + 0.5f) / m_hdri.getHeight());
			auto wi_object = geometry::SphericalCoordinate(1.0f, uv.y * glm::pi<float>(), uv.x * glm::two_pi<float>()).toCartesianCoordinate();
			light_sample.le = getLeObjectSpace(wi_object, glm::vec3(0.0f), 0.0f);
			light_sample.pdf_w = getPdfObjectSpace(wi_object, glm::vec3(0.0f), 0.0f);
			light_sample.wi_world = glm::normalize(m_transformation.vectorToWorldSpace(wi_object));
			light_sample.distance = std::numeric_limits<float>::max();

			return light_sample;
		}

		LightSample EnvironmentLight::getVisibleSample(const core::Scene& scene, const geometry::Ray& ray) const
		{
			light::LightSample light_sample;
			if (!scene.intersectShadowRay(ray, std::numeric_limits<float>::max()))
			{
				light_sample.wi_world = ray.get_direction();
				light_sample.le = getLe(light_sample.wi_world, glm::vec3(0.0f), 0.0f);
				light_sample.pdf_w = getPdf(light_sample.wi_world, glm::vec3(0.0f), 0.0f);
				light_sample.distance = std::numeric_limits<float>::max();
			}

			return light_sample;
		}

		glm::vec3 EnvironmentLight::getLe(const glm::vec3& wi_world, const glm::vec3& light_plane_normal, float distance) const
		{
			auto uv = geometry::SphericalMapper().mapOnlyUV(m_transformation.vectorToObjectSpace(wi_world), glm::vec3(0.0f)).uv;

			return m_hdri.fetchTexelNearest(uv, 0);
		}

		float EnvironmentLight::getPdf(const glm::vec3& wi_world, const glm::vec3& light_plane_normal, float distance) const
		{
			auto uv = geometry::SphericalMapper().mapOnlyUV(m_transformation.vectorToObjectSpace(wi_world), glm::vec3(0.0f)).uv;
			auto i = static_cast<int>(uv.x * m_hdri.getWidth() - 0.5f);
			auto j = static_cast<int>(uv.y * m_hdri.getHeight() - 0.5f);

			return m_hdri.getWidth() * m_hdri.getHeight() * m_sampler.getPdf(i, j) / (glm::two_pi<float>() * glm::pi<float>() * glm::sin(uv.y * glm::pi<float>()));
		}

		bool EnvironmentLight::hasDeltaDistribution() const
		{
			return false;
		}

		std::shared_ptr<geometry::Object> EnvironmentLight::getObject() const
		{
			return nullptr;
		}

		glm::vec3 EnvironmentLight::getLeObjectSpace(const glm::vec3& wi_object, const glm::vec3& light_plane_normal, float distance) const
		{
			auto uv = geometry::SphericalMapper().mapOnlyUV(wi_object, glm::vec3(0.0f)).uv;

			return m_hdri.fetchTexelNearest(uv, 0);
		}

		float EnvironmentLight::getPdfObjectSpace(const glm::vec3& wi_object, const glm::vec3& light_plane_normal, float distance) const
		{
			auto uv = geometry::SphericalMapper().mapOnlyUV(wi_object, glm::vec3(0.0f)).uv;
			auto i = static_cast<int>(uv.x * m_hdri.getWidth() - 0.5f);
			auto j = static_cast<int>(uv.y * m_hdri.getHeight() - 0.5f);

			return m_hdri.getWidth() * m_hdri.getHeight() * m_sampler.getPdf(i, j) / (glm::two_pi<float>() * glm::pi<float>() * glm::sin(uv.y * glm::pi<float>()));

		}
	}
}