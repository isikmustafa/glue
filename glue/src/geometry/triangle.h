#ifndef __GLUE__GEOMETRY__TRIANGLE__
#define __GLUE__GEOMETRY__TRIANGLE__

#include "bbox.h"
#include "..\core\uniform_sampler.h"

#include <glm\vec2.hpp>
#include <glm\vec3.hpp>
#include <array>

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

			glm::vec3 samplePoint(core::UniformSampler& sampler);
			float getSurfaceArea() const;
			BBox getBBox() const;
			glm::vec2 getBBoxOnAxis(int axis) const;
			std::array<glm::vec3, 3> getVertices() const;
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