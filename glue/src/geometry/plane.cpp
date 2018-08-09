#include "plane.h"
#include "ray.h"

#include <glm\geometric.hpp>

namespace glue
{
	namespace geometry
	{
		glm::vec3 Plane::intersect(const Ray& ray)
		{
			auto distance = (glm::dot(normal, point) - glm::dot(normal, ray.get_origin())) / glm::dot(normal, ray.get_direction());
			return ray.getPoint(distance);
		}
	}
}