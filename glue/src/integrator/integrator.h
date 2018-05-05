#ifndef __GLUE__INTEGRATOR__INTEGRATOR__
#define __GLUE__INTEGRATOR__INTEGRATOR__

#include "..\core\forward_decl.h"

#include <glm\vec3.hpp>

namespace glue
{
	namespace integrator
	{
		class Integrator
		{
		public:
			virtual ~Integrator() {}

			virtual glm::vec3 integratePixel(const core::Scene& scene, int x, int y) const = 0;
		};
	}
}

#endif