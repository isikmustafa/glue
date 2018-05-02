#ifndef __GLUE__MATERIAL__DIELECTRIC__
#define __GLUE__MATERIAL__DIELECTRIC__

#include "bsdf_material.h"
#include "..\microfacet\microfacet_scattering.h"
#include "..\microfacet\fresnel.h"
#include "..\microfacet\ggx_distribution.h"

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
			Dielectric(float ior_n, float roughness);

			std::pair<glm::vec3, glm::vec3> sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler) const override;
			glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const override;
			float getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const override;
			bool hasDeltaDistribution() const override;
			bool useMultipleImportanceSampling() const override;

		private:
			float m_ior_n;
			microfacet::MicrofacetScattering<microfacet::GGXDistribution> m_microfacet;
		};
	}
}

#endif