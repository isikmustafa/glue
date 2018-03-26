#ifndef __GLUE__GEOMETRY__MESH__
#define __GLUE__GEOMETRY__MESH__

#include "bbox.h"

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
			Mesh(const BBox& bbox, const std::shared_ptr<std::vector<Triangle>>& triangles, const std::shared_ptr<BVH>& bvh);

			BBox getBBox() const;
			bool intersect(const Ray& ray, Intersection& intersection, float max_distance) const;
			bool intersectShadowRay(const Ray& ray, float max_distance) const;

		private:
			BBox m_bbox; //will be useful when transformations are implemented.
			std::shared_ptr<std::vector<Triangle>> m_triangles;
			std::shared_ptr<BVH> m_bvh;
		};
	}
}

#endif