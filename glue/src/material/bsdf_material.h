#ifndef __GLUE__MATERIAL__BSDFMATERIAL__
#define __GLUE__MATERIAL__BSDFMATERIAL__

#include "..\core\uniform_sampler.h"
#include "..\geometry\spherical_coordinate.h"

#include <glm\vec3.hpp>

namespace glue
{
	namespace material
	{
		class BsdfMaterial
		{
		public:
			virtual ~BsdfMaterial() {}

			//wi, wo should be in tangent space for getBsdf, getPdf and sampleWi
			//This makes it easier and efficient to calculate some computations.
			virtual geometry::SphericalCoordinate sampleWi(const glm::vec3& wo_tangent, core::UniformSampler& sampler) const = 0;
			virtual glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const = 0;
			virtual float getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const = 0;
			virtual bool hasDeltaDistribution() const = 0;
			virtual bool useMultipleImportanceSampling() const = 0;
		};
	}
}

#endif