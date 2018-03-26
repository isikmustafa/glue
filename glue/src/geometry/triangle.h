#ifndef __GLUE__GEOMETRY__TRIANGLE__
#define __GLUE__GEOMETRY__TRIANGLE__

#include "bbox.h"

#include <glm\vec3.hpp>

namespace glue
{
	namespace geometry
	{
		class Ray;
		struct Intersection;
		class Triangle
		{
		public:
			Triangle(const glm::vec3& v0, const glm::vec3& edge1, const glm::vec3& edge2);

			BBox getBBox() const;
			bool intersect(const Ray& ray, Intersection& intersection, float max_distance) const;
			bool intersectShadowRay(const Ray& ray, float max_distance) const;

		private:
			glm::vec3 m_v0;
			glm::vec3 m_edge1;
			glm::vec3 m_edge2;
			glm::vec3 m_normal;
		};
	}
}

#endif