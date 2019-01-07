#ifndef __GLUE__LIGHT__ENVIRONMENTLIGHT__
#define __GLUE__LIGHT__ENVIRONMENTLIGHT__

#include "light.h"
#include "../texture/image_texture.h"
#include "../geometry/sphere.h"
#include "../geometry/transformation.h"
#include "../core/discrete_2d_sampler.h"

namespace glue
{
	namespace light
	{
		class EnvironmentLight : public Light
		{
		public:
			//Xml structure of the class.
			struct Xml : public Light::Xml
			{
				texture::ImageTexture::Xml hdri;
				geometry::Transformation::Xml transformation;

				explicit Xml(const xml::Node& node);
				std::unique_ptr<Light> create() const override;
			};

		public:
			explicit EnvironmentLight(const EnvironmentLight::Xml& xml);

			Photon castPhoton(core::UniformSampler& sampler) const override { return Photon(); }
			LightSample sample(core::UniformSampler& sampler, const geometry::Intersection& intersection) const override;
			LightSample getVisibleSample(const core::Scene& scene, const geometry::Ray& ray) const override;
			glm::vec3 getLe(const glm::vec3& wi_world, const glm::vec3& light_plane_normal, float distance) const override;
			float getPdf(const glm::vec3& wi_world, const glm::vec3& light_plane_normal, float distance) const override;
			bool hasDeltaDistribution() const override;
			std::shared_ptr<geometry::Object> getObject() const override;

		private:
			texture::ImageTexture m_hdri;
			core::Discrete2DSampler m_sampler;
			geometry::Transformation m_transformation;

		private:
			glm::vec3 getLeObjectSpace(const glm::vec3& wi_object, const glm::vec3& light_plane_normal, float distance) const;
			float getPdfObjectSpace(const glm::vec3& wi_object, const glm::vec3& light_plane_normal, float distance) const;
		};
	}
}

#endif