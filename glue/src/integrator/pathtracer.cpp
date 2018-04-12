#include "pathtracer.h"
#include "..\geometry\intersection.h"
#include "..\core\gaussian_sampler.h"
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
				auto ray = scene.camera.castPrimayRay(x, y, offset_sampler->sample(), offset_sampler->sample());
				pixel_acc += estimateLi(scene, ray, uniform_sampler, 1.0f, false);
			}

			return glm::clamp(255.0f * pixel_acc / static_cast<float>(scene.sample_count), 0.0f, 255.0f);
		}

		glm::vec3 Pathtracer::estimateLi(const core::Scene& scene, const geometry::Ray& ray,
			core::UniformSampler& uniform_sampler, float importance, bool light_explicitly_sampled) const
		{
			//Russian roulette.
			if (importance < 0.2f)
			{
				if (uniform_sampler.sample() > importance)
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
			auto basis = geometry::OrthonormalBasis(intersection.normal);
			geometry::SphericalCoordinate wo_spherical(-ray.get_direction(), basis);

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
					if (intersection.bsdf_material->useMultipleImportanceSampling() && !light->hasDeltaDistribution())
					{

					}
					else
					{
						auto sampled_plane = light->samplePlane(uniform_sampler);
						auto wi_cartesian = sampled_plane.point - intersection_point;
						auto distance = glm::length(wi_cartesian);
						wi_cartesian /= distance;

						auto cos_light_surface = glm::dot(-wi_cartesian, sampled_plane.normal);
						if (cos_light_surface <= 0.0f)
						{
							continue;
						}

						geometry::Ray shadow_ray(intersection_point + wi_cartesian * scene.secondary_ray_epsilon, wi_cartesian);
						if (!scene.bvh.intersectShadowRay(scene.meshes, shadow_ray, distance - 1.1f * scene.secondary_ray_epsilon))
						{
							geometry::SphericalCoordinate wi_spherical(wi_cartesian, basis);
							auto bsdf = intersection.bsdf_material->getBsdf(wi_spherical, wo_spherical);
							auto cos = glm::max(0.0f, glm::dot(intersection.normal, wi_cartesian)); //can be converted to glm::cos(wi_spherical.theta)

							//Get p(A) and transform it to p(w)
							auto pdf = light->getPdf();
							pdf *= distance * distance / cos_light_surface;

							direct_lo += (bsdf * cos / pdf) * light->getLe();
						}
					}
				}

				light_explicitly_sampled = true;
			}
			else
			{
				light_explicitly_sampled = false;
			}

			//INDIRECT LIGHTING//
			geometry::SphericalCoordinate wi_spherical(intersection.bsdf_material->sampleDirection(uniform_sampler));

			auto bsdf = intersection.bsdf_material->getBsdf(wi_spherical, wo_spherical);
			auto pdf = intersection.bsdf_material->getPdf(wi_spherical, wo_spherical);

			auto wi_cartesian = wi_spherical.convertToCartesian(basis);
			auto cos = glm::max(0.0f, glm::dot(intersection.normal, wi_cartesian)); //can be converted to glm::cos(wi_spherical.theta)

			geometry::Ray wi_ray(intersection_point + wi_cartesian * scene.secondary_ray_epsilon, wi_cartesian);

			auto f = bsdf * cos / pdf;
			auto indirect_lo = f * estimateLi(scene, wi_ray, uniform_sampler, importance * glm::max(glm::max(f.x, f.y), f.z), light_explicitly_sampled);

			return importance < 0.2f ? (direct_lo + indirect_lo) / importance : direct_lo + indirect_lo;
		}
	}
}