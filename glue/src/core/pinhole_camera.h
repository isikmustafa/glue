#ifndef __GLUE__CORE__PINHOLECAMERA__
#define __GLUE__CORE__PINHOLECAMERA__

#include "coordinate_space.h"
#include "..\geometry\ray.h"

#include <glm\vec2.hpp>
#include <glm\vec3.hpp>
#include <glm\vec4.hpp>

namespace glue
{
	namespace core
	{
		class PinholeCamera
		{
		public:
			PinholeCamera(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up,
				const glm::vec4& screen_coordinates, const glm::ivec2& screen_resolution, float near_distance);

			geometry::Ray castPrimayRay(int x, int y, float offset_x = 0.5f, float offset_y = 0.5f) const;

			const glm::ivec2& get_screen_resolution() const { return m_screen_resolution; }

		private:
			CoordinateSpace m_camera_space;
			glm::vec4 m_screen_coordinates;
			glm::ivec2 m_screen_resolution;
			glm::vec2 m_pixel_length;
			float m_near_distance;
		};
	}
}

#endif