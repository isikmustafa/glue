#include "triangle.h"
#include "ray.h"
#include "intersection.h"

#include <glm\geometric.hpp>

namespace glue
{
	namespace geometry
	{
		Triangle::Triangle(const glm::vec3& v0, const glm::vec3& edge1, const glm::vec3& edge2)
			: m_v0(v0)
			, m_edge1(edge1)
			, m_edge2(edge2)
			, m_normal(glm::normalize(glm::cross(edge1, edge2)))
		{}

		geometry::Plane Triangle::samplePlane(core::UniformSampler& sampler) const
		{
			auto v1 = m_edge1 + m_v0;
			auto v2 = m_edge2 + m_v0;

			auto sqrt_rand = glm::sqrt(sampler.sample());
			auto u = 1.0f - sqrt_rand;
			auto v = sampler.sample() * sqrt_rand;

			return geometry::Plane(m_v0 * u + v1 * v + v2 * (1.0f - u - v), m_normal);
		}

		float Triangle::getSurfaceArea() const
		{
			return glm::length(glm::cross(m_edge1, m_edge2)) * 0.5f;
		}

		BBox Triangle::getBBox() const
		{
			auto v1 = m_edge1 + m_v0;
			auto v2 = m_edge2 + m_v0;

			return BBox(glm::min(glm::min(v1, v2), m_v0), glm::max(glm::max(v1, v2), m_v0));
		}

		glm::vec2 Triangle::getBoundsOnAxis(int axis) const
		{
			auto v1 = m_edge1 + m_v0;
			auto v2 = m_edge2 + m_v0;

			return glm::vec2(glm::min(glm::min(v1[axis], v2[axis]), m_v0[axis]), glm::max(glm::max(v1[axis], v2[axis]), m_v0[axis]));
		}

		std::array<glm::vec3, 3> Triangle::getVertices() const
		{
			return { m_v0, m_edge1 + m_v0, m_edge2 + m_v0 };
		}

		bool Triangle::intersect(const Ray& ray, Intersection& intersection, float max_distance) const
		{
			auto pvec = glm::cross(ray.get_direction(), m_edge2);
			auto inv_det = 1.0f / glm::dot(m_edge1, pvec);

			auto tvec = ray.get_origin() - m_v0;
			auto w1 = glm::dot(tvec, pvec) * inv_det;

			auto qvec = glm::cross(tvec, m_edge1);
			auto w2 = glm::dot(ray.get_direction(), qvec) * inv_det;

			if (w1 < 0.0f || w2 < 0.0f || (w1 + w2) > 1.0f)
			{
				return false;
			}

			auto distance = glm::dot(m_edge2, qvec) * inv_det;
			if (distance > 0.0f && distance < max_distance)
			{
				intersection.plane.normal = m_normal;
				intersection.distance = distance;

				return true;
			}
			return false;
		}

		bool Triangle::intersectShadowRay(const Ray& ray, float max_distance) const
		{
			auto pvec = glm::cross(ray.get_direction(), m_edge2);
			auto inv_det = 1.0f / glm::dot(m_edge1, pvec);

			auto tvec = ray.get_origin() - m_v0;
			auto w1 = glm::dot(tvec, pvec) * inv_det;

			auto qvec = glm::cross(tvec, m_edge1);
			auto w2 = glm::dot(ray.get_direction(), qvec) * inv_det;

			if (w1 < 0.0f || w2 < 0.0f || (w1 + w2) > 1.0f)
			{
				return false;
			}

			auto distance = glm::dot(m_edge2, qvec) * inv_det;
			return distance > 0.0f && distance < max_distance;
		}
	}
}