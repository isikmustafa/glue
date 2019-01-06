#ifndef __GLUE__CORE__PINHOLECAMERA__
#define __GLUE__CORE__PINHOLECAMERA__

#include "coordinate_space.h"
#include "../geometry/ray.h"
#include "../core/forward_decl.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace glue
{
	namespace core
	{
		class PinholeCamera
		{
		public:
			//Xml structure of the class.
			struct Xml
			{
				glm::vec3 position;
				glm::vec3 direction;
				glm::vec3 up;
				glm::vec2 fov_xy;
				glm::ivec2 resolution;
				float near_distance;

				explicit Xml(const xml::Node& node);
			};

		public:
			explicit PinholeCamera(const PinholeCamera::Xml& xml);

			geometry::Ray castRay(int x, int y, float offset_x = 0.5f, float offset_y = 0.5f) const;

			const glm::ivec2& get_resolution() const { return m_resolution; }

		private:
			CoordinateSpace m_camera_space;
			glm::vec4 m_screen_coordinates;
			glm::ivec2 m_resolution;
			glm::vec2 m_pixel_length;
			float m_near_distance;
		};
	}
}

#endif