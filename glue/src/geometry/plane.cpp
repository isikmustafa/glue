#include "plane.h"
#include "ray.h"

#include <glm\geometric.hpp>

namespace glue
{
	namespace geometry
	{
		Plane::Plane(const glm::vec3& p_point, const glm::vec3& p_normal)
			: point(p_point)
			, normal(p_normal)
		{}

		glm::vec3 Plane::intersect(const Ray& ray)
		{
			auto distance = (glm::dot(normal, point) - glm::dot(normal, ray.get_origin())) / glm::dot(normal, ray.get_direction());
			return ray.getPoint(distance);
		}
	}
}