#ifndef __GLUE__GEOMETRY__INTERSECTION__
#define __GLUE__GEOMETRY__INTERSECTION__

#include "plane.h"
#include "../core/forward_decl.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace glue
{
	namespace geometry
	{
		struct Intersection
		{
			Plane plane;
			glm::vec2 uv;
			glm::vec3 dpdu;
			glm::vec3 dpdv;
			float distance{ -1.0f };
			int bsdf_choice{ -1 };
			//Pointers will be kept as raw pointers.
			//The reason is to prevent overhead due to shared_ptr destruction and construction
			//and to protect the ownership of the pointers kept by unique_ptrs.
			const Object* object{ nullptr };
			const Triangle* triangle{ nullptr };
			const material::BsdfMaterial* bsdf_material{ nullptr };
			bool radiance_transport{ true };
		};
	}
}

#endif