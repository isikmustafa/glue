#include "mesh.h"
#include "triangle.h"
#include "bvh.h"

#include <algorithm>

namespace glue
{
	namespace geometry
	{
		//cdf is not a reference argument in order to prevent copying.
		//It is moved along the way but not copied.
		Mesh::Mesh(const Transformation& transformation, const BBox& bbox, std::vector<float> cdf, float area,
			const std::shared_ptr<std::vector<Triangle>>& triangles, const std::shared_ptr<BVH>& bvh)
			: m_transformation(transformation)
			, m_bbox(bbox)
			, m_cdf(std::move(cdf))
			, m_area(area)
			, m_triangles(triangles)
			, m_bvh(bvh)
		{}

		glm::vec3 Mesh::samplePoint(core::UniformSampler& sampler) const
		{
			auto x = sampler.sample() * m_area;
			auto itr = std::upper_bound(m_cdf.begin(), m_cdf.end(), x);
			auto index = std::distance(m_cdf.begin(), itr);

			auto vertices = (*m_triangles)[index].getVertices();
			auto v0 = m_transformation.pointToWorldSpace(vertices[0]);
			auto v1 = m_transformation.pointToWorldSpace(vertices[1]);
			auto v2 = m_transformation.pointToWorldSpace(vertices[2]);

			return Triangle(v0, v1 - v0, v2 - v0).samplePoint(sampler);
		}

		//This function practically serves as a getter.
		//However, the convention for both triangle and mesh classes should be the same.
		float Mesh::getSurfaceArea() const
		{
			return m_area;
		}

		//This function practically serves as a getter.
		//However, the convention for both triangle and mesh classes should be the same.
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
				intersection.normal = glm::normalize(m_transformation.normalToWorldSpace(intersection.normal));
				intersection.mesh = this;
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