#ifndef __GLUE__INTEGRATOR__RAYTRACER__
#define __GLUE__INTEGRATOR__RAYTRACER__

#include "integrator.h"

namespace glue
{
	namespace integrator
	{
		class Raytracer : public Integrator
		{
		public:
			glm::vec3 integratePixel(const core::Scene& scene, int x, int y) const override;
		};
	}
}

#endif