#include "mesh.h"
#include "triangle.h"
#include "bvh.h"

#include <algorithm>

namespace glue
{
	namespace geometry
	{
		Mesh::Mesh(const Transformation& transformation, const BBox& bbox, const std::vector<float>& triangle_areas, float area,
			const std::shared_ptr<BVH<Triangle>>& bvh, std::unique_ptr<material::BsdfMaterial> bsdf_material)
			: m_transformation(transformation)
			, m_bbox(bbox)
			, m_triangle_sampler(triangle_areas)
			, m_area(area)
			, m_bvh(bvh)
			, m_bsdf_material(std::move(bsdf_material))
		{}

		geometry::Plane Mesh::samplePlane(core::UniformSampler& sampler)
		{
			auto vertices = m_bvh->get_objects()[m_triangle_sampler.sample(sampler)].getVertices();
			auto v0 = m_transformation.pointToWorldSpace(vertices[0]);
			auto v1 = m_transformation.pointToWorldSpace(vertices[1]);
			auto v2 = m_transformation.pointToWorldSpace(vertices[2]);

			return Triangle(v0, v1 - v0, v2 - v0).samplePlane(sampler);
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

		glm::vec2 Mesh::getBoundsOnAxis(int axis) const
		{
			return glm::vec2(m_bbox.get_min()[axis], m_bbox.get_max()[axis]);
		}

		bool Mesh::intersect(const Ray& ray, Intersection& intersection, float max_distance) const
		{
			if (m_bvh->intersect(m_transformation.rayToObjectSpace(ray), intersection, max_distance))
			{
				intersection.plane.point = ray.getPoint(intersection.distance);
				intersection.plane.normal = glm::normalize(m_transformation.normalToWorldSpace(intersection.plane.normal));
				intersection.mesh = this;
				intersection.bsdf_material = m_bsdf_material.get();
				return true;
			}
			return false;
		}

		bool Mesh::intersectShadowRay(const Ray& ray, float max_distance) const
		{
			return m_bvh->intersectShadowRay(m_transformation.rayToObjectSpace(ray), max_distance);
		}
	}
}