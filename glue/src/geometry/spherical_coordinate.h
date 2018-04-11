#ifndef __GLUE__GEOMETRY__SPHERICALCOORDINATE__
#define __GLUE__GEOMETRY__SPHERICALCOORDINATE__

#include <glm\vec3.hpp>

namespace glue
{
	namespace geometry
	{
		struct OrthonormalBasis
		{
			glm::vec3 u;
			glm::vec3 v;
			glm::vec3 w;

			OrthonormalBasis(const glm::vec3& p_w);
			OrthonormalBasis(const glm::vec3& p_u, const glm::vec3& p_v, const glm::vec3& p_w);
		};

		struct SphericalCoordinate
		{
			float radius;
			float theta;
			float phi;

			SphericalCoordinate(float p_radius, float p_theta, float p_phi);

			glm::vec3 convertToCartesian(const OrthonormalBasis& basis) const;
		};
	}
}

#endif