#ifndef __GLUE__GEOMETRY__SPHERICALCOORDINATE__
#define __GLUE__GEOMETRY__SPHERICALCOORDINATE__

#include "orthonormal_basis.h"

#include <glm\vec3.hpp>

namespace glue
{
	namespace geometry
	{
		struct SphericalCoordinate
		{
			float radius;
			float theta;
			float phi;

			SphericalCoordinate(const glm::vec3& direction, const OrthonormalBasis& basis);
			SphericalCoordinate(float p_radius, float p_theta, float p_phi);

			glm::vec3 convertToCartesian(const OrthonormalBasis& basis) const;
		};
	}
}

#endif