#include "ray.h"

namespace glue
{
	namespace geometry
	{
		Ray::Ray(const glm::vec3& origin, const glm::vec3& direction)
			: m_origin(origin)
			, m_direction(direction)
		{}

		glm::vec3 Ray::getPoint(float distance) const
		{
			return m_origin + m_direction * distance;
		}
	}
}