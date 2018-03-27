#include "mesh.h"
#include "triangle.h"
#include "bvh.h"

namespace glue
{
	namespace geometry
	{
		Mesh::Mesh(const Transformation& transformation, const BBox& bbox, const std::shared_ptr<std::vector<Triangle>>& triangles, const std::shared_ptr<BVH>& bvh)
			: m_transformation(transformation)
			, m_bbox(bbox)
			, m_triangles(triangles)
			, m_bvh(bvh)
		{}

		//This function practically serves as a getter.
		//However, when building bvh with template build function, "getBBox" is necessary.
		BBox Mesh::getBBox() const
		{
			return m_bbox;
		}

		glm::vec2 Mesh::getBBoxOnAxis(int axis) const
		{
			return glm::vec2(m_bbox.get_min()[axis], m_bbox.get_max()[axis]);
		}

		bool Mesh::intersect(const Ray& ray, Intersection& intersection, float max_distance) const
		{
			if (m_bvh->intersect(*m_triangles, m_transformation.rayToObjectSpace(ray), intersection, max_distance))
			{
				intersection.normal = m_transformation.normalToWorldSpace(intersection.normal);
				return true;
			}
			return false;
		}

		bool Mesh::intersectShadowRay(const Ray& ray, float max_distance) const
		{
			return m_bvh->intersectShadowRay(*m_triangles, m_transformation.rayToObjectSpace(ray), max_distance);
		}
	}
}