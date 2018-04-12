#ifndef __GLUE__LIGHT__LIGHT__
#define __GLUE__LIGHT__LIGHT__

#include "..\geometry\ray.h"
#include "..\geometry\plane.h"
#include "..\core\uniform_sampler.h"

#include <glm\vec3.hpp>

namespace glue
{
	namespace light
	{
		//This class is not designed to abstract every possible light.
		//Its functions and function signatures may change over time.
		class Light
		{
		public:
			virtual ~Light() {}

			virtual geometry::Plane samplePlane(core::UniformSampler& sampler) const = 0;
			virtual glm::vec3 getLe() const = 0;
			virtual float getPdf() const = 0;
			virtual bool hasDeltaDistribution() const = 0;
		};
	}
}

#endif