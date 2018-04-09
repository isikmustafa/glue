#include "raytracer.h"
#include "..\geometry\intersection.h"
#include "..\core\uniform_sampler.h"
#include "..\core\gaussian_sampler.h"

#include <limits>
#include <glm\geometric.hpp>

namespace glue
{
	namespace integrator
	{
		glm::vec3 Raytracer::integratePixel(const core::Scene& scene, int x, int y) const
		{
			std::unique_ptr<core::Sampler> sampler;
			if (scene.pixel_filter == "BOX")
			{
				sampler.reset(new core::UniformSampler(0.0f, 1.0f));
			}
			else if (scene.pixel_filter == "GAUSSIAN")
			{
				sampler.reset(new core::GaussianSampler(0.5f, 0.5f));
			}

			glm::vec3 pixel_acc;
			for (int i = 0; i < scene.sample_count; ++i)
			{
				auto ray = scene.camera.castPrimayRay(x, y, sampler->sample(), sampler->sample());
				geometry::Intersection intersection;
				auto result = scene.bvh.intersect(scene.meshes, ray, intersection, std::numeric_limits<float>::max());

				if (result)
				{
					pixel_acc += glm::dot(intersection.normal, -ray.get_direction()) * glm::vec3(255.0f);
				}
				else
				{
					pixel_acc += scene.background_color;
				}
			}

			return pixel_acc / static_cast<float>(scene.sample_count);
		}
	}
}