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

			//p_w is assumed to be normalized.
			OrthonormalBasis(const glm::vec3& p_w);
			//p_u, p_v and p_w are assumed to establish an orthonormal basis.
			OrthonormalBasis(const glm::vec3& p_u, const glm::vec3& p_v, const glm::vec3& p_w);
		};

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