#ifndef __GLUE__INTEGRATOR__PATHTRACER__
#define __GLUE__INTEGRATOR__PATHTRACER__

#include "..\core\scene.h"
#include "..\core\uniform_sampler.h"

#include <glm\vec3.hpp>

namespace glue
{
	namespace integrator
	{
		class Pathtracer
		{
		public:
			glm::vec3 integratePixel(const core::Scene& scene, int x, int y) const;

		private:
			glm::vec3 estimate(const core::Scene& scene, const geometry::Ray& ray,
				core::UniformSampler& uniform_sampler, float importance, bool light_explicitly_sampled) const;
		};
	}
}

#endif