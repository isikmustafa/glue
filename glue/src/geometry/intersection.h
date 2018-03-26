#ifndef __GLUE__GEOMETRY__INTERSECTION__
#define __GLUE__GEOMETRY__INTERSECTION__

#include <glm\vec3.hpp>

namespace glue
{
	namespace geometry
	{
		struct Intersection
		{
			glm::vec3 normal;
			float distance;
		};
	}
}

#endif