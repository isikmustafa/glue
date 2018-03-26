#include "camera.h"

#include <glm\geometric.hpp>

namespace glue
{
	namespace core
	{
		Camera::Camera(const glm::vec4& screen_coordinates, const glm::vec3& position, const glm::vec3& direction,
			const glm::vec3& up, const glm::ivec2& screen_resolution, float near_distance)
			: m_screen_coordinates(screen_coordinates)
			, m_position(position)
			, m_gaze(-direction)
			, m_up(up)
			, m_screen_resolution(screen_resolution)
			, m_near_distance(near_distance)
		{
			m_gaze = glm::normalize(m_gaze);
			m_right = glm::normalize(glm::cross(m_up, m_gaze));
			m_up = glm::normalize(glm::cross(m_gaze, m_right));

			m_coeff.x = (m_screen_coordinates.y - m_screen_coordinates.x) / m_screen_resolution.x;
			m_coeff.y = (m_screen_coordinates.w - m_screen_coordinates.z) / m_screen_resolution.y;
		}

		geometry::Ray Camera::castPrimayRay(int x, int y) const
		{
			auto dir = -m_near_distance * m_gaze +
				(m_screen_coordinates.x + m_coeff.x * (static_cast<float>(x) + 0.5f)) * m_right + (m_screen_coordinates.w - m_coeff.y * (static_cast<float>(y) + 0.5f)) * m_up;

			return geometry::Ray(m_position, glm::normalize(dir));
		}
	}
}