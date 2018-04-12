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
			glm::vec3 getBsdf(const geometry::SphericalCoordinate& wi, const geometry::SphericalCoordinate& wo) const override;
			float getPdf(const geometry::SphericalCoordinate& wi, const geometry::SphericalCoordinate& wo) const override;
			bool hasDeltaDistribution() const override;
			bool useMultipleImportanceSampling() const override;

		private:
			glm::vec3 m_kd;
		};
	}
}

#endif