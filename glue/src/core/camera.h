#ifndef __GLUE__CORE__CAMERA__
#define __GLUE__CORE__CAMERA__

#include "..\geometry\ray.h"

#include <glm\vec2.hpp>
#include <glm\vec3.hpp>
#include <glm\vec4.hpp>

namespace glue
{
	namespace core
	{
		class Camera
		{
		public:
			Camera() = default;
			Camera(const glm::vec4& screen_coordinates, const glm::vec3& position, const glm::vec3& direction,
				const glm::vec3& up, const glm::ivec2& screen_resolution, float near_distance);

			geometry::Ray castPrimayRay(int x, int y, float offset_x = 0.5f, float offset_y = 0.5f) const;

			const glm::ivec2& get_screen_resolution() const { return m_screen_resolution; }

		private:
			glm::vec4 m_screen_coordinates;
			glm::vec3 m_position;
			glm::vec3 m_gaze;
			glm::vec3 m_up;
			glm::vec3 m_right;
			glm::vec2 m_coeff;
			glm::ivec2 m_screen_resolution;
			float m_near_distance;
		};
	}
}

#endif