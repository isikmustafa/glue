#ifndef __GLUE__GEOMETRY__RAY__
#define __GLUE__GEOMETRY__RAY__

#include <glm\vec3.hpp>
#include <memory>

namespace glue
{
	namespace geometry
	{
		class Ray
		{
		public:
			Ray() = default;
			Ray(const glm::vec3& origin, const glm::vec3& direction);

			glm::vec3 getPoint(float distance) const;

			const glm::vec3& get_origin() const { return m_origin; }
			const glm::vec3& get_direction() const { return m_direction; }

		private:
			glm::vec3 m_origin;
			glm::vec3 m_direction;
		};
	}
}

#endif