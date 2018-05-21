#ifndef __GLUE__GEOMETRY__INTERSECTION__
#define __GLUE__GEOMETRY__INTERSECTION__

#include "plane.h"
#include "..\core\forward_decl.h"

namespace glue
{
	namespace geometry
	{
		struct Intersection
		{
			Plane plane;
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