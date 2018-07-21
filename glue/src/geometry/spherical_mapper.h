#ifndef __GLUE__GEOMETRY__SPHERICALMAPPER__
#define __GLUE__GEOMETRY__SPHERICALMAPPER__

#include <glm\vec2.hpp>
#include <glm\vec3.hpp>

namespace glue
{
	namespace geometry
	{
		struct SphericalMapper
		{
			glm::vec2 uv;
			glm::vec3 dpdu;
			glm::vec3 dpdv;

			SphericalMapper(const glm::vec3& point);
		};
	}
}

#endif