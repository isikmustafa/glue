#include "sphere.h"
#include "ray.h"
#include "intersection.h"

#include <glm\geometric.hpp>

namespace glue
{
	namespace geometry
	{
		Sphere::Sphere(const Transformation& transformation, const BBox& bbox, float area, std::unique_ptr<material::BsdfMaterial> bsdf_material)
			: m_transformation(transformation)
			, m_bbox(bbox)
			, m_area(area)
			, m_bsdf_material(std::move(bsdf_material))
		{}

		geometry::Plane Sphere::samplePlane(core::UniformSampler& sampler)
		{
			//Uniform sample the sphere
			//m_transformation.pointToWorldSpace
			//m_transformation.normalToWorldSpace
			//return plane
			return geometry::Plane();
		}

		float Sphere::getSurfaceArea() const
		{
			return m_area;
		}

		BBox Sphere::getBBox() const
		{
			return m_bbox;
		}

		glm::vec2 Sphere::getBoundsOnAxis(int axis) const
		{
			return glm::vec2(m_bbox.get_min()[axis], m_bbox.get_max()[axis]);
		}

		bool Sphere::intersect(const Ray& ray, Intersection& intersection, float max_distance) const
		{
			//Analytic solution.
			auto transformed_ray = m_transformation.rayToObjectSpace(ray);
			auto a = glm::dot(transformed_ray.get_direction(), transformed_ray.get_direction());
			auto b = 2 * glm::dot(transformed_ray.get_direction(), transformed_ray.get_origin());
			auto c = glm::dot(transformed_ray.get_origin(), transformed_ray.get_origin()) - 1.0f;

			auto distance = 0.0f;
			auto discriminant = b * b - 4 * a * c;
			if (discriminant >= 0.0f)
			{
				auto delta = glm::sqrt(discriminant);
				auto div = 0.5f / a; //To prevent cancelling out.
				distance = -div * b - div * delta;

				if (distance < 0.0f)
				{
					distance = -div * b + div * delta;
				}
			}

			if (distance > 0.0f && distance < max_distance)
			{
				intersection.plane.point = ray.getPoint(distance);
				intersection.plane.normal = glm::normalize(m_transformation.normalToWorldSpace(transformed_ray.getPoint(distance)));
				intersection.distance = distance;
				intersection.object = this;
				intersection.bsdf_material = m_bsdf_material.get();

				return true;
			}

			return false;
		}

		bool Sphere::intersectShadowRay(const Ray& ray, float max_distance) const
		{
			//Analytic solution.
			auto transformed_ray = m_transformation.rayToObjectSpace(ray);
			auto a = glm::dot(transformed_ray.get_direction(), transformed_ray.get_direction());
			auto b = 2 * glm::dot(transformed_ray.get_direction(), transformed_ray.get_origin());
			auto c = glm::dot(transformed_ray.get_origin(), transformed_ray.get_origin()) - 1.0f;

			auto distance = 0.0f;
			auto discriminant = b * b - 4 * a * c;
			if (discriminant >= 0.0f)
			{
				auto delta = glm::sqrt(discriminant);
				auto div = 0.5f / a; //To prevent cancelling out.
				distance = -div * b - div * delta;

				if (distance < 0.0f)
				{
					distance = -div * b + div * delta;
				}
			}

			return (distance > 0.0f && distance < max_distance);
		}
	}
}