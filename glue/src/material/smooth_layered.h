#ifndef __GLUE__MATERIAL__SMOOTHLAYERED__
#define __GLUE__MATERIAL__SMOOTHLAYERED__

#include "bsdf_material.h"
#include "..\texture\texture.h"

namespace glue
{
	namespace material
	{
		class SmoothLayered : public BsdfMaterial
		{
		public:
			//Xml structure of the class.
			struct Xml : public BsdfMaterial::Xml
			{
				std::unique_ptr<texture::Texture::Xml> kd;
				float ior_n;

				explicit Xml(const xml::Node& node);
				Xml(std::unique_ptr<texture::Texture::Xml> p_kd, float p_ior_n);
				std::unique_ptr<BsdfMaterial> create() const override;
			};

		public:
			SmoothLayered(const SmoothLayered::Xml& xml);

			int chooseBsdf(const glm::vec3& wi_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const override;
			std::pair<glm::vec3, glm::vec3> sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const override;
			glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const override;
			float getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const override;
			bool hasDeltaDistribution(const geometry::Intersection& intersection) const override;
			bool useMultipleImportanceSampling(const geometry::Intersection& intersection) const override;

		private:
			std::unique_ptr<texture::Texture> m_kd;
			float m_ior_n;
			float m_fsum;
		};
	}
}

#endif