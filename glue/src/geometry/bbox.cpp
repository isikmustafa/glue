#include "bbox.h"

#include <glm\common.hpp>
#include <limits>

namespace glue
{
	namespace geometry
	{
		BBox::BBox()
			: m_min(std::numeric_limits<float>::max())
			, m_max(-std::numeric_limits<float>::max())
		{}

		BBox::BBox(const glm::vec3& min, const glm::vec3& max)
			: m_min(min)
			, m_max(max)
		{}

		void BBox::extend(const glm::vec3& point)
		{
			m_min = glm::min(m_min, point);
			m_max = glm::max(m_max, point);
		}

		void BBox::extend(const BBox& bbox)
		{
			m_min = glm::min(m_min, bbox.m_min);
			m_max = glm::max(m_max, bbox.m_max);
		}

		float BBox::getSurfaceArea() const
		{
			auto edges = m_max - m_min;
			return 2 * (edges.x * edges.y + edges.x * edges.z + edges.y * edges.z);
		}

		glm::vec2 BBox::intersect(const glm::vec3& origin, const glm::vec3& inv_dir) const
		{
			auto t0 = (m_min - origin) * inv_dir;
			auto t1 = (m_max - origin) * inv_dir;

			auto tm_min = glm::min(t0.x, t1.x);
			auto tm_max = glm::max(t0.x, t1.x);

			tm_min = glm::max(tm_min, glm::min(t0.y, t1.y));
			tm_max = glm::min(tm_max, glm::max(t0.y, t1.y));

			tm_min = glm::max(tm_min, glm::min(t0.z, t1.z));
			tm_max = glm::min(tm_max, glm::max(t0.z, t1.z));

			if (tm_max < tm_min)
			{
				return glm::vec2(-1.0f, -1.0f);
			}

			//Return both the result and parameter of the closest intersection point which is tm_min.
			//The reason why tm_min is also returned is to reject traversing a node whose tm_min is greater than min_distance of that traversal.
			return tm_min > 0.0f ? glm::vec2(tm_min, tm_min) : glm::vec2(tm_max, tm_min);
		}
	}
}