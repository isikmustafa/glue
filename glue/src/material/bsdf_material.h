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

			virtual geometry::SphericalCoordinate sampleDirection(core::UniformSampler& sampler) const = 0;
			virtual glm::vec3 getBsdf(const geometry::SphericalCoordinate& wi, const geometry::SphericalCoordinate& wo) const = 0;
			virtual float getPdf(const geometry::SphericalCoordinate& wi, const geometry::SphericalCoordinate& wo) const = 0;
			virtual bool hasDeltaDistribution() const = 0;
			virtual bool useMultipleImportanceSampling() const = 0;
		};
	}
}

#endif