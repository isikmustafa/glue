#ifndef __GLUE__GEOMETRY__MESH__
#define __GLUE__GEOMETRY__MESH__

#include "bbox.h"
#include "transformation.h"

#include <memory>
#include <vector>

namespace glue
{
	namespace geometry
	{
		class BVH;
		class Triangle;
		class Ray;
		struct Intersection;
		class Mesh
		{
		public:
			Mesh(const Transformation& transformation, const BBox& bbox, const std::shared_ptr<std::vector<Triangle>>& triangles, const std::shared_ptr<BVH>& bvh);

			BBox getBBox() const;
			glm::vec2 getBBoxOnAxis(int axis) const;
			bool intersect(const Ray& ray, Intersection& intersection, float max_distance) const;
			bool intersectShadowRay(const Ray& ray, float max_distance) const;

		private:
			Transformation m_transformation;
			BBox m_bbox;
			std::shared_ptr<std::vector<Triangle>> m_triangles;
			std::shared_ptr<BVH> m_bvh;
		};
	}
}

#endif