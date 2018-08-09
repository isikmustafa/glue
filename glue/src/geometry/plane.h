#ifndef __GLUE__GEOMETRY__PLANE__
#define __GLUE__GEOMETRY__PLANE__

#include "..\core\forward_decl.h"

#include <glm\vec3.hpp>

namespace glue
{
	namespace geometry
	{
		struct Plane
		{
			glm::vec3 point;
			glm::vec3 normal;

			//p_normal is assumed to be normalized.
			Plane() = default;
			Plane(const glm::vec3& p_point, const glm::vec3& p_normal)
				: point(p_point)
				, normal(p_normal)
			{}

			//Returns the point intersected on the plane.
			glm::vec3 intersect(const Ray& ray);
		};
	}
}

#endif