#include "pathtracer.h"
#include "..\geometry\intersection.h"
#include "..\core\gaussian_sampler.h"
#include "..\core\coordinate_space.h"
#include "..\light\diffuse_arealight.h"

#include <limits>
#include <glm\geometric.hpp>

namespace glue
{
	namespace integrator
	{
		glm::vec3 Pathtracer::integratePixel(const core::Scene& scene, int x, int y) const
		{
			std::unique_ptr<core::RealSampler> offset_sampler;
			if (scene.pixel_filter == core::Filter::BOX)
			{
				offset_sampler = std::make_unique<core::UniformSampler>(0.0f, 1.0f);
			}
			else if (scene.pixel_filter == core::Filter::GAUSSIAN)
			{
				offset_sampler = std::make_unique<core::GaussianSampler>(0.5f, 0.5f);
			}

			glm::vec3 pixel_acc;
			core::UniformSampler uniform_sampler;
			for (int i = 0; i < scene.sample_count; ++i)
			{
				auto ray = scene.camera->castPrimayRay(x, y, offset_sampler->sample(), offset_sampler->sample());
				pixel_acc += estimateLi(scene, ray, uniform_sampler, 1.0f, false);
			}

			return pixel_acc / static_cast<float>(scene.sample_count);
		}

		glm::vec3 Pathtracer::estimateLi(const core::Scene& scene, const geometry::Ray& ray,
			core::UniformSampler& uniform_sampler, float importance, bool light_explicitly_sampled) const
		{
			//Russian roulette.
			if (importance < 0.2f)
			{
				if (uniform_sampler.sample() > 0.5f)
				{
					return glm::vec3(0.0f);
				}
			}

			geometry::Intersection intersection;
			auto is_intersected = scene.bvh.intersect(scene.meshes, ray, intersection, std::numeric_limits<float>::max());
			if (!is_intersected)
			{
				return scene.background_color;
			}

			//Check if the ray hits a light source.
			auto itr = scene.light_meshes.find(intersection.mesh);
			if (itr != scene.light_meshes.end())
			{
				if (!light_explicitly_sampled && glm::dot(-ray.get_direction(), intersection.normal) > 0.0f)
				{
					return itr->second->getLe();
				}
				else
				{
					return glm::vec3(0.0f);
				}
			}

			auto intersection_point = ray.getPoint(intersection.distance);
			core::CoordinateSpace tangent_space(intersection_point, intersection.normal);
			auto wo_tangent = tangent_space.vectorToLocalSpace(-ray.get_direction());

			//DIRECT LIGHTING//
			glm::vec3 direct_lo(0.0f);
			//If the material does not have a delta pdf, then estimate light directly.
			if (!intersection.bsdf_material->hasDeltaDistribution())
			{
				int size = scene.lights.size();
				for (int i = 0; i < size; ++i)
				{
					const auto* light = scene.lights[i].get();
					//TODO: MIS
					/*if (intersection.bsdf_material->useMultipleImportanceSampling() && !light->hasDeltaDistribution())
					{

					}*/
					auto sampled_plane = light->samplePlane(uniform_sampler);
					auto wi_world = sampled_plane.point - intersection_point;
					auto distance = glm::length(wi_world);
					wi_world /= distance;

					auto cos_light_surface = glm::dot(-wi_world, sampled_plane.normal);
					if (cos_light_surface <= 0.0f)
					{
						continue;
					}

					geometry::Ray shadow_ray(intersection_point + wi_world * scene.secondary_ray_epsilon, wi_world);
					if (!scene.bvh.intersectShadowRay(scene.meshes, shadow_ray, distance - 1.1f * scene.secondary_ray_epsilon))
					{
						auto wi_tangent = tangent_space.vectorToLocalSpace(wi_world);
						auto bsdf = intersection.bsdf_material->getBsdf(wi_tangent, wo_tangent);
						auto cos = material::cosTheta(wi_tangent);

						//Get p(A) and transform it to p(w)
						auto pdf = light->getPdf();
						pdf *= distance * distance / cos_light_surface;

						direct_lo += (bsdf * cos / pdf) * light->getLe();
					}
				}

				light_explicitly_sampled = true;
			}
			else
			{
				light_explicitly_sampled = false;
			}

			//INDIRECT LIGHTING//
			auto wi_tangent = intersection.bsdf_material->sampleWi(wo_tangent, uniform_sampler);

			auto bsdf = intersection.bsdf_material->getBsdf(wi_tangent, wo_tangent);
			auto cos = material::cosTheta(wi_tangent);
			auto pdf = intersection.bsdf_material->getPdf(wi_tangent, wo_tangent);

			auto f = bsdf * cos / pdf;

			glm::vec3 indirect_lo(0.0f);
			if (f.x + f.y + f.z > 0.0f)
			{
				auto wi_world = tangent_space.vectorToWorldSpace(wi_tangent);
				geometry::Ray wi_ray(intersection_point + wi_world * scene.secondary_ray_epsilon, wi_world);

				indirect_lo = f * estimateLi(scene, wi_ray, uniform_sampler, importance * glm::max(glm::max(f.x, f.y), f.z), light_explicitly_sampled);
			}

			return importance < 0.2f ? (direct_lo + indirect_lo) * 2.0f : direct_lo + indirect_lo;
		}
	}
}