#include "ray.h"

namespace glue
{
	namespace geometry
	{
		Ray::Ray(const glm::vec3& origin, const glm::vec3& direction)
			: m_origin(origin)
			, m_direction(direction)
			, m_rx(nullptr)
			, m_ry(nullptr)
		{}

		Ray::Ray(const glm::vec3 & origin, const glm::vec3 & direction, std::unique_ptr<Ray> rx, std::unique_ptr<Ray> ry)
			: m_origin(origin)
			, m_direction(direction)
			, m_rx(std::move(rx))
			, m_ry(std::move(ry))
		{}

		glm::vec3 Ray::getPoint(float distance) const
		{
			return m_origin + m_direction * distance;
		}
	}
}