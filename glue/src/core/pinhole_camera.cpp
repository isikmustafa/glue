#include "pinhole_camera.h"
#include "..\xml\node.h"

#include <glm\geometric.hpp>
#include <glm\trigonometric.hpp>

namespace glue
{
	namespace core
	{
		PinholeCamera::Xml::Xml(const xml::Node& node)
		{
			node.parseChildText("Position", &position.x, &position.y, &position.z);
			node.parseChildText("Direction", &direction.x, &direction.y, &direction.z);
			node.parseChildText("Up", &up.x, &up.y, &up.z);
			node.parseChildText("FovXY", &fov_xy.x, &fov_xy.y);
			node.parseChildText("Resolution", &resolution.x, &resolution.y);
			node.parseChildText("NearDistance", &near_distance);
		}

		PinholeCamera::PinholeCamera(const PinholeCamera::Xml& xml)
			: m_camera_space(xml.position, -xml.direction, xml.up)
			, m_resolution(xml.resolution)
			, m_near_distance(xml.near_distance)
		{
			auto s_right = glm::tan(glm::radians(xml.fov_xy.x) * 0.5f) * xml.near_distance;
			auto s_up = glm::tan(glm::radians(xml.fov_xy.y) * 0.5f) * xml.near_distance;

			m_screen_coordinates = glm::vec4(-s_right, s_right, -s_up, s_up);
			m_pixel_length = glm::vec2((m_screen_coordinates.y - m_screen_coordinates.x) / m_resolution.x, (m_screen_coordinates.w - m_screen_coordinates.z) / m_resolution.y);
		}

		geometry::Ray PinholeCamera::castPrimayRay(int x, int y, float offset_x, float offset_y) const
		{
			glm::vec3 pixel_coordinate(m_screen_coordinates.x + m_pixel_length.x * (static_cast<float>(x) + offset_x),
				m_screen_coordinates.w - m_pixel_length.y * (static_cast<float>(y) + offset_y), -m_near_distance);

			return geometry::Ray(m_camera_space.get_origin(), glm::normalize(m_camera_space.vectorToWorldSpace(pixel_coordinate)),
				std::make_unique<geometry::Ray>(m_camera_space.get_origin(), glm::normalize(m_camera_space.vectorToWorldSpace(pixel_coordinate + glm::vec3(m_pixel_length.x, 0.0f, 0.0f)))),
				std::make_unique<geometry::Ray>(m_camera_space.get_origin(), glm::normalize(m_camera_space.vectorToWorldSpace(pixel_coordinate + glm::vec3(0.0f, -m_pixel_length.y, 0.0f)))));
		}
	}
}