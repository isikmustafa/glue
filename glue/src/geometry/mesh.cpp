#include "mesh.h"
#include "triangle.h"
#include "bvh.h"

namespace glue
{
	namespace geometry
	{
		Mesh::Mesh(const BBox& bbox, const std::shared_ptr<std::vector<Triangle>>& triangles, const std::shared_ptr<BVH>& bvh)
			: m_bbox(bbox)
			, m_triangles(triangles)
			, m_bvh(bvh)
		{}

		//This function practically serves as a getter.
		//However, when building bvh with template build function, "getBBox" is necessary.
		BBox Mesh::getBBox() const
		{
			return m_bbox;
		}

		bool Mesh::intersect(const Ray& ray, Intersection& intersection, float max_distance) const
		{
			return m_bvh->intersect(*m_triangles, ray, intersection, max_distance);
		}

		bool Mesh::intersectShadowRay(const Ray& ray, float max_distance) const
		{
			return m_bvh->intersectShadowRay(*m_triangles, ray, max_distance);
		}
	}
}