#ifndef __GLUE__MATERIAL__DIELECTRIC__
#define __GLUE__MATERIAL__DIELECTRIC__

#include "bsdf_material.h"
#include "../microfacet/microfacet_scattering.h"
#include "../microfacet/fresnel.h"
#include "../microfacet/ggx_distribution.h"
#include "../microfacet/beckmann_distribution.h"
#include "../texture/texture.h"

namespace glue
{
	namespace material
	{
		//Works for Dielectric<->Dielectric boundary.
		//Outside dielectric is assumed to be air and thus IOR is equal to 1.
		//It can be extended so that other dielectric materials can be used at the boundary.
		class Dielectric : public BsdfMaterial
		{
		public:
			//Xml structure of the class.
			struct Xml : public BsdfMaterial::Xml
			{
				float ior_n;
				std::unique_ptr<texture::Texture::Xml> roughness;

				explicit Xml(const xml::Node& node);
				Xml(float p_ior_n, std::unique_ptr<texture::Texture::Xml> p_roughness);
				std::unique_ptr<BsdfMaterial> create() const override;
			};

		public:
			explicit Dielectric(const Dielectric::Xml& xml);

			std::pair<glm::vec3, glm::vec3> sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const override;
			glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const override;
			float getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const override;
			bool hasDeltaDistribution(const geometry::Intersection& intersection) const override;
			bool useMultipleImportanceSampling(const geometry::Intersection& intersection) const override;

		private:
			float m_ior_n;
			std::unique_ptr<texture::Texture> m_roughness;
		};
	}
}

#endif