#ifndef __GLUE__MATERIAL__ORENNAYAR__
#define __GLUE__MATERIAL__ORENNAYAR__

#include "bsdf_material.h"
#include "../texture/texture.h"

namespace glue
{
	namespace material
	{
		class OrenNayar : public BsdfMaterial
		{
		public:
			//Xml structure of the class.
			struct Xml : public BsdfMaterial::Xml
			{
				std::unique_ptr<texture::Texture::Xml> kd;
				float roughness;

				explicit Xml(const xml::Node& node);
				Xml(std::unique_ptr<texture::Texture::Xml> p_kd, float p_roughness);
				std::unique_ptr<BsdfMaterial> create() const override;
			};

		public:
			explicit OrenNayar(const OrenNayar::Xml& xml);

			std::pair<glm::vec3, glm::vec3> sampleWi(const glm::vec3& wo_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const override;
			glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const override;
			float getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const override;
			bool hasDeltaDistribution(const geometry::Intersection& intersection) const override;
			bool useMultipleImportanceSampling(const geometry::Intersection& intersection) const override;

		private:
			std::unique_ptr<texture::Texture> m_kd;
			float m_A;
			float m_B;
		};
	}
}

#endif