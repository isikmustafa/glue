#ifndef __GLUE__MATERIAL__METAL__
#define __GLUE__MATERIAL__METAL__

#include "bsdf_material.h"
#include "..\microfacet\microfacet_reflection.h"
#include "..\microfacet\fresnel.h"
#include "..\microfacet\ggx_distribution.h"

namespace glue
{
	namespace material
	{
		class Metal : public BsdfMaterial
		{
		public:
			Metal(const glm::vec3& n_ratio, const glm::vec3& k_ratio, float roughness);

			glm::vec3 sampleWi(const glm::vec3& wo_tangent, core::UniformSampler& sampler) const override;
			glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const override;
			float getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const override;
			bool hasDeltaDistribution() const override;
			bool useMultipleImportanceSampling() const override;

		private:
			microfacet::MicrofacetReflection<microfacet::fresnel::Conductor, microfacet::GGXDistribution> m_microfacet;
		};
	}
}

#endif