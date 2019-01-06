#ifndef __GLUE__MATERIAL__LAMBERTIAN__
#define __GLUE__MATERIAL__LAMBERTIAN__

#include "bsdf_material.h"
#include "../texture/texture.h"

namespace glue
{
	namespace material
	{
		class Lambertian : public BsdfMaterial
		{
		public:
			//Xml structure of the class.
			struct Xml : public BsdfMaterial::Xml
			{
				std::unique_ptr<texture::Texture::Xml> kd;

				explicit Xml(const xml::Node& node);
				explicit Xml(std::unique_ptr<texture::Texture::Xml> p_kd);
				std::unique_ptr<BsdfMaterial> create() const override;
			};

		public:
			explicit Lambertian(const Lambertian::Xml& xml);

			std::pair<glm::vec3, glm::vec3> sampleWi(const glm::vec3& wo_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const override;
			glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const override;
			float getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const override;
			bool hasDeltaDistribution(const geometry::Intersection& intersection) const override;
			bool useMultipleImportanceSampling(const geometry::Intersection& intersection) const override;
			bool isSpecular(const geometry::Intersection& intersection) const override;

		private:
			std::unique_ptr<texture::Texture> m_kd;
		};
	}
}

#endif