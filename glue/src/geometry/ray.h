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
			Ray(const glm::vec3& origin, const glm::vec3& direction);
			Ray(const glm::vec3& origin, const glm::vec3& direction, std::unique_ptr<Ray> rx, std::unique_ptr<Ray> ry);

			glm::vec3 getPoint(float distance) const;

			const glm::vec3& get_origin() const { return m_origin; }
			const glm::vec3& get_direction() const { return m_direction; }
			const Ray* get_rx() const { return m_rx.get(); }
			const Ray* get_ry() const { return m_ry.get(); }

		private:
			glm::vec3 m_origin;
			glm::vec3 m_direction;
			std::unique_ptr<Ray> m_rx;
			std::unique_ptr<Ray> m_ry;
		};
	}
}

#endif