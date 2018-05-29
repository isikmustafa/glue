#ifndef __GLUE__MATERIAL__METAL__
#define __GLUE__MATERIAL__METAL__

#include "bsdf_material.h"
#include "..\microfacet\microfacet_reflection.h"
#include "..\microfacet\fresnel.h"
#include "..\microfacet\ggx_distribution.h"
#include "..\microfacet\beckmann_distribution.h"

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
				float roughness;

				explicit Xml(const xml::Node& node);
				Xml(const glm::vec3& p_ior_n, const glm::vec3& p_ior_k, float p_roughness);
				std::unique_ptr<BsdfMaterial> create() const override;
			};

		public:
			Metal(const Metal::Xml& xml);

			std::pair<glm::vec3, glm::vec3> sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler) const override;
			glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const override;
			float getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const override;
			bool hasDeltaDistribution() const override;
			bool useMultipleImportanceSampling() const override;

		private:
			glm::vec3 m_ior_n;
			glm::vec3 m_ior_k;
			microfacet::MicrofacetReflection<microfacet::GGXDistribution> m_microfacet;
			bool m_use_mis;
		};
	}
}

#endif