#ifndef __GLUE__GEOMETRY__INTERSECTION__
#define __GLUE__GEOMETRY__INTERSECTION__

#include "..\material\bsdf_material.h"

#include <glm\vec3.hpp>

namespace glue
{
	namespace geometry
	{
		class Mesh;
		struct Intersection
		{
			//normal should always be normalized.
			glm::vec3 normal;
			float distance;
			//Pointers will be kept as raw pointers.
			//The reason is to prevent overhead due to shared_ptr destruction and construction
			//and to protect the ownership of the pointers kept by unique_ptrs.
			const Mesh* mesh;
			const material::BsdfMaterial* bsdf_material;
		};
	}
}

#endif