#ifndef __GLUE__GEOMETRY__DEBUGSPHERE__
#define __GLUE__GEOMETRY__DEBUGSPHERE__

#include "bbox.h"

#include <glm\vec2.hpp>
#include <glm\vec3.hpp>

namespace glue
{
	namespace geometry
	{
		class Ray;
		struct Intersection;
		class DebugSphere
		{
		public:
			DebugSphere(const glm::vec3& center, float radius);

			BBox getBBox() const;
			glm::vec2 getBBoxOnAxis(int axis) const;
			bool intersect(const Ray& ray, Intersection& intersection, float max_distance) const;
			bool intersectShadowRay(const Ray& ray, float max_distance) const;

		private:
			glm::vec3 m_center;
			float m_radius;
		};
	}
}

#endif