#include "raytracer.h"
#include "..\geometry\intersection.h"

#include <limits>
#include <glm\geometric.hpp>

namespace glue
{
	namespace integrator
	{
		glm::vec3 Raytracer::integratePixel(const core::Scene& scene, int x, int y) const
		{
			auto ray = scene.camera.castPrimayRay(x, y);
			geometry::Intersection intersection;
			auto result = scene.bvh.intersect(scene.meshes, ray, intersection, std::numeric_limits<float>::max());

			if (result)
			{
				return (1.0f - glm::dot(intersection.normal, ray.get_direction())) * glm::vec3(255.0f);
			}
			else
			{
				return scene.background_color;
			}
		}
	}
}