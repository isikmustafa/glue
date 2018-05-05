#include "pathtracer.h"
#include "..\geometry\intersection.h"
#include "..\core\real_sampler.h"
#include "..\core\coordinate_space.h"
#include "..\core\scene.h"
#include "..\light\diffuse_arealight.h"

#include <limits>
#include <glm\geometric.hpp>

namespace glue
{
	namespace integrator
	{
		Pathtracer::Pathtracer(std::unique_ptr<core::Filter> filter, int sample_count)
			: m_filter(std::move(filter))
			, m_sample_count(sample_count)
		{}

		glm::vec3 Pathtracer::integratePixel(const core::Scene& scene, int x, int y) const
		{
			auto offset_sampler = m_filter->generateSampler();
			core::UniformSampler uniform_sampler;

			glm::vec3 pixel_acc;
			for (int i = 0; i < m_sample_count; ++i)
			{
				auto ray = scene.camera->castPrimayRay(x, y, offset_sampler->sample(), offset_sampler->sample());
				pixel_acc += estimate(scene, ray, uniform_sampler, 1.0f, false);
			}

			return pixel_acc / static_cast<float>(m_sample_count);
		}

		glm::vec3 Pathtracer::estimate(const core::Scene& scene, const geometry::Ray& ray,
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
			auto wi_tangent = tangent_space.vectorToLocalSpace(-ray.get_direction());

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
					auto wo_world = sampled_plane.point - intersection_point;
					auto distance = glm::length(wo_world);
					wo_world /= distance;

					auto wo_tangent = tangent_space.vectorToLocalSpace(wo_world);

					auto bsdf = intersection.bsdf_material->getBsdf(wi_tangent, wo_tangent);
					auto cos = glm::abs(material::cosTheta(wo_tangent));

					//Get p(A) and transform it to p(w)
					auto pdf = light->getPdf();
					pdf *= distance * distance / glm::dot(-wo_world, sampled_plane.normal);

					auto f = bsdf * cos / pdf;

					//One other important thing about this if check is that it never does a computation for NAN values of f.
					if (f.x + f.y + f.z > 0.0f)
					{
						geometry::Ray shadow_ray(intersection_point + wo_world * scene.secondary_ray_epsilon, wo_world);
						if (!scene.bvh.intersectShadowRay(scene.meshes, shadow_ray, distance - 1.1f * scene.secondary_ray_epsilon))
						{
							direct_lo += f * light->getLe();
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
			auto w_f = intersection.bsdf_material->sampleWo(wi_tangent, uniform_sampler);

			const auto& wo_tangent = w_f.first;
			const auto& f = w_f.second;

			glm::vec3 indirect_lo(0.0f);
			//One other important thing about this if check is that it never does a computation for NAN values of f.
			if (f.x + f.y + f.z > 0.0f)
			{
				auto wo_world = tangent_space.vectorToWorldSpace(wo_tangent);
				geometry::Ray wo_ray(intersection_point + wo_world * scene.secondary_ray_epsilon, wo_world);

				indirect_lo = f * estimate(scene, wo_ray, uniform_sampler, importance * glm::max(glm::max(f.x, f.y), f.z), light_explicitly_sampled);
			}

			return importance < 0.2f ? (direct_lo + indirect_lo) * 2.0f : direct_lo + indirect_lo;
		}
	}
}