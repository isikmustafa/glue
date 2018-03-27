#ifndef __GLUE__GEOMETRY__BBOX__
#define __GLUE__GEOMETRY__BBOX__

#include <glm\vec3.hpp>

namespace glue
{
	namespace geometry
	{
		class BBox
		{
		public:
			BBox();
			BBox(const glm::vec3& min, const glm::vec3& max);

			void extend(const glm::vec3& point);
			void extend(const BBox& bbox);
			float getSurfaceArea() const;
			glm::vec2 intersect(const glm::vec3& origin, const glm::vec3& inv_dir) const;

			const glm::vec3& get_min() const { return m_min; }
			const glm::vec3& get_max() const { return m_max; }

		private:
			glm::vec3 m_min;
			glm::vec3 m_max;
		};
	}
}

#endif