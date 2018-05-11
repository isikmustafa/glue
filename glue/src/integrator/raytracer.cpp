#include "raytracer.h"
#include "..\geometry\intersection.h"
#include "..\core\real_sampler.h"
#include "..\core\scene.h"

#include <limits>
#include <glm\geometric.hpp>

namespace glue
{
	namespace integrator
	{
		glm::vec3 Raytracer::integratePixel(const core::Scene& scene, int x, int y) const
		{
			auto ray = scene.camera->castPrimayRay(x, y);
			geometry::Intersection intersection;
			auto result = scene.bvh.intersect(scene.meshes, ray, intersection, std::numeric_limits<float>::max());

			if (scene.debug_bvh.intersectShadowRay(scene.debug_spheres, ray, intersection.distance))
			{
				return glm::vec3(1.0f, 0.0f, 0.0f);
			}
			else if (result)
			{
				return glm::vec3(glm::dot(intersection.normal, -ray.get_direction()));
			}
			else
			{
				return scene.background_radiance;
			}
		}
	}
}