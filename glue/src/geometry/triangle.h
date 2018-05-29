#ifndef __GLUE__GEOMETRY__TRIANGLE__
#define __GLUE__GEOMETRY__TRIANGLE__

#include "bbox.h"
#include "plane.h"
#include "..\core\forward_decl.h"
#include "..\core\real_sampler.h"

#include <glm\vec2.hpp>
#include <glm\vec3.hpp>
#include <array>

namespace glue
{
	namespace geometry
	{
		//Although having almost the same member functions, this class do not inherit from Object base class.
		//The reason is that Triangle is not designed to have its own material and transformation.
		//Triangles exist only for building a mesh.
		class Triangle
		{
		public:
			Triangle(const glm::vec3& v0, const glm::vec3& edge1, const glm::vec3& edge2);

			geometry::Plane samplePlane(core::UniformSampler& sampler) const;
			float getSurfaceArea() const;
			BBox getBBox() const;
			glm::vec2 getBoundsOnAxis(int axis) const;
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