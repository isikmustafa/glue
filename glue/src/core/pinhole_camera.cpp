#include "pinhole_camera.h"

#include <glm\geometric.hpp>

namespace glue
{
	namespace core
	{
		PinholeCamera::PinholeCamera(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up,
			const glm::vec4& screen_coordinates, const glm::ivec2& screen_resolution, float near_distance)
			: m_camera_space(position, -direction, up)
			, m_screen_coordinates(screen_coordinates)
			, m_screen_resolution(screen_resolution)
			, m_pixel_length((m_screen_coordinates.y - m_screen_coordinates.x) / m_screen_resolution.x, (m_screen_coordinates.w - m_screen_coordinates.z) / m_screen_resolution.y)
			, m_near_distance(near_distance)
		{}

		geometry::Ray PinholeCamera::castPrimayRay(int x, int y, float offset_x, float offset_y) const
		{
			glm::vec3 pixel_coordinate(m_screen_coordinates.x + m_pixel_length.x * (static_cast<float>(x) + offset_x),
				m_screen_coordinates.w - m_pixel_length.y * (static_cast<float>(y) + offset_y), -m_near_distance);

			return geometry::Ray(m_camera_space.get_origin(), glm::normalize(m_camera_space.vectorToWorldSpace(pixel_coordinate)));
		}
	}
}