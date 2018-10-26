#ifndef __GLUE__MATERIAL__METAL__
#define __GLUE__MATERIAL__METAL__

#include "bsdf_material.h"
#include "..\microfacet\microfacet_reflection.h"
#include "..\microfacet\fresnel.h"
#include "..\microfacet\ggx_distribution.h"
#include "..\microfacet\beckmann_distribution.h"
#include "..\texture\texture.h"

namespace glue
{
	namespace material
	{
		//Works for Dielectric<->Metal boundary.
		//Dielectric is assumed to be air and thus IOR of outside is equal to 1.
		//It can be extended so that other dielectric materials can be used at the boundary.
		class Metal : public BsdfMaterial
		{
		public:
			//Xml structure of the class.
			struct Xml : public BsdfMaterial::Xml
			{
				glm::vec3 ior_n;
				glm::vec3 ior_k;
				std::unique_ptr<texture::Texture::Xml> roughness;

				explicit Xml(const xml::Node& node);
				Xml(const glm::vec3& p_ior_n, const glm::vec3& p_ior_k, std::unique_ptr<texture::Texture::Xml> p_roughness);
				std::unique_ptr<BsdfMaterial> create() const override;
			};

		public:
			Metal(const Metal::Xml& xml);

			std::pair<glm::vec3, glm::vec3> sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const override;
			glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const override;
			float getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const override;
			bool hasDeltaDistribution(const geometry::Intersection& intersection) const override;
			bool useMultipleImportanceSampling(const geometry::Intersection& intersection) const override;

		private:
			glm::vec3 m_ior_n;
			glm::vec3 m_ior_k;
			std::unique_ptr<texture::Texture> m_roughness;
		};
	}
}

#endif