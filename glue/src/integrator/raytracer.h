#ifndef __GLUE__INTEGRATOR__RAYTRACER__
#define __GLUE__INTEGRATOR__RAYTRACER__

#include "..\core\scene.h"

#include <glm\vec3.hpp>

namespace glue
{
	namespace integrator
	{
		class Raytracer
		{
		public:
			glm::vec3 integratePixel(const core::Scene& scene, int x, int y) const;
		};
	}
}

#endif