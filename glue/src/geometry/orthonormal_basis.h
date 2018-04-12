#ifndef __GLUE__GEOMETRY__ORTHONORMALBASIS__
#define __GLUE__GEOMETRY__ORTHONORMALBASIS__

#include <glm\vec3.hpp>
#include <glm\geometric.hpp>

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
			OrthonormalBasis(const glm::vec3& p_w)
				: u(glm::normalize(glm::cross(glm::abs(p_w.x) > 0.1f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f), p_w)))
				, v(glm::cross(p_w, u))
				, w(p_w)
			{}
			//p_u, p_v and p_w are assumed to establish an orthonormal basis.
			OrthonormalBasis(const glm::vec3& p_u, const glm::vec3& p_v, const glm::vec3& p_w)
				: u(p_u)
				, v(p_v)
				, w(p_w)
			{}
		};
	}
}

#endif