#include "debug_sphere.h"
#include "ray.h"
#include "intersection.h"

#include <glm\geometric.hpp>

namespace glue
{
	namespace geometry
	{
		DebugSphere::DebugSphere(const glm::vec3& center, float radius)
			: m_center(center)
			, m_radius(radius)
		{}

		BBox DebugSphere::getBBox() const
		{
			return BBox(m_center - glm::vec3(m_radius), m_center + glm::vec3(m_radius));
		}

		glm::vec2 DebugSphere::getBoundsOnAxis(int axis) const
		{
			return glm::vec2(m_center[axis] - m_radius, m_center[axis] + m_radius);
		}

		bool DebugSphere::intersect(const Ray& ray, Intersection& intersection, float max_distance) const
		{
			auto e = m_center - ray.get_origin();
			auto a = glm::dot(e, ray.get_direction());
			auto x = m_radius * m_radius + a * a - glm::dot(e, e);

			if (x < 0)
			{
				return false;
			}

			auto distance = a - glm::sqrt(x);
			if (distance > 0.0f && distance < max_distance)
			{
				intersection.plane.point = ray.getPoint(distance);
				intersection.plane.normal = glm::normalize(ray.getPoint(distance) - m_center);
				intersection.distance = distance;
				return true;
			}
			return false;
		}

		bool DebugSphere::intersectShadowRay(const Ray& ray, float max_distance) const
		{
			auto e = m_center - ray.get_origin();
			auto a = glm::dot(e, ray.get_direction());
			auto x = m_radius * m_radius + a * a - glm::dot(e, e);

			if (x < 0)
			{
				return false;
			}

			auto distance = a - glm::sqrt(x);
			return distance > 0.0f && distance < max_distance;
		}
	}
}