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

			std::pair<glm::vec3, glm::vec3> sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler) const override;
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