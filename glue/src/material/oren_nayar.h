#ifndef __GLUE__MATERIAL__ORENNAYAR__
#define __GLUE__MATERIAL__ORENNAYAR__

#include "bsdf_material.h"

namespace glue
{
	namespace material
	{
		class OrenNayar : public BsdfMaterial
		{
		public:
			OrenNayar(const glm::vec3& kd, float roughness_in_radians);

			//wi, wo should be in tangent space for getBsdf, getPdf and sampleWi
			//This makes it easier and efficient to calculate some computations.
			geometry::SphericalCoordinate sampleWi(const glm::vec3& wo_tangent, core::UniformSampler& sampler) const override;
			glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const override;
			float getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const override;
			bool hasDeltaDistribution() const override;
			bool useMultipleImportanceSampling() const override;

		private:
			glm::vec3 m_kd;
			float m_A;
			float m_B;
		};
	}
}

#endif