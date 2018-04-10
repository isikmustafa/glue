#ifndef __GLUE__GEOMETRY__INTERSECTION__
#define __GLUE__GEOMETRY__INTERSECTION__

#include <glm\vec3.hpp>

namespace glue
{
	namespace geometry
	{
		class Mesh;
		struct Intersection
		{
			glm::vec3 normal;
			float distance;
			//Pointers will be kept as raw pointers.
			//The reason is to prevent overhead due to shared_ptr destruction and construction.
			const Mesh* mesh;
			//const Material* material;
		};
	}
}

#endif