#ifndef __GLUE__MATERIAL__LAMBERTIAN__
#define __GLUE__MATERIAL__LAMBERTIAN__

#include "bsdf_material.h"

namespace glue
{
	namespace material
	{
		class Lambertian : public BsdfMaterial
		{
		public:
			Lambertian(const glm::vec3& kd);

			geometry::SphericalCoordinate sampleDirection(core::UniformSampler& sampler) const override;
			//wi and wo should be in tangent space for both getBsdf and getPdf.
			//This makes it easier and efficient to calculate some computations.
			glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const override;
			float getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const override;
			bool hasDeltaDistribution() const override;
			bool useMultipleImportanceSampling() const override;

		private:
			glm::vec3 m_kd;
		};
	}
}

#endif