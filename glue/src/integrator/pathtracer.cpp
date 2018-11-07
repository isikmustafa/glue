#include "pathtracer.h"
#include "..\geometry\intersection.h"
#include "..\core\real_sampler.h"
#include "..\core\coordinate_space.h"
#include "..\core\scene.h"
#include "..\core\math.h"
#include "..\light\light.h"
#include "..\material\bsdf_material.h"
#include "..\xml\node.h"

#include <limits>
#include <glm\geometric.hpp>

namespace glue
{
	namespace integrator
	{
		Pathtracer::Xml::Xml(const xml::Node& node)
		{
			filter = core::Filter::Xml::factory(node.child("Filter", true));
			node.parseChildText("SampleCount", &sample_count);
			node.parseChildText("RRThreshold", &rr_threshold);
		}

		std::unique_ptr<Integrator> Pathtracer::Xml::create() const
		{
			return std::make_unique<Pathtracer>(*this);
		}

		Pathtracer::Pathtracer(const Pathtracer::Xml& xml)
			: m_filter(xml.filter->create())
			, m_sample_count(xml.sample_count)
			, m_rr_threshold(xml.rr_threshold)
		{}

		glm::vec3 Pathtracer::integratePixel(const core::Scene& scene, int x, int y) const
		{
			auto offset_sampler = m_filter->generateSampler();
			core::UniformSampler uniform_sampler;

			glm::vec3 pixel_acc(0.0f);
			for (int i = 0; i < m_sample_count; ++i)
			{
				auto ray = scene.camera->castPrimayRay(x, y, offset_sampler->sample(), offset_sampler->sample());
				pixel_acc *= (static_cast<float>(i) / (i + 1));
				pixel_acc += 1.0f / (i + 1) * estimate(scene, ray, uniform_sampler, 1.0f, false);
			}

			return pixel_acc;
		}

		glm::vec3 Pathtracer::estimate(const core::Scene& scene, const geometry::Ray& ray,
			core::UniformSampler& uniform_sampler, float importance, bool light_explicitly_sampled) const
		{
			constexpr float cutoff_probability = 0.5f;
			constexpr float calc_weight = 1.0f / (1.0f - cutoff_probability);

			//Russian roulette.
			if (importance < m_rr_threshold)
			{
				if (uniform_sampler.sample() < cutoff_probability)
				{
					return glm::vec3(0.0f);
				}
			}

			geometry::Intersection intersection;
			if (!scene.bvh.intersect(ray, intersection, std::numeric_limits<float>::max()))
			{
				auto le = scene.getBackgroundRadiance(ray.get_direction(), light_explicitly_sampled);
				return importance < m_rr_threshold ? le * calc_weight : le;
			}

			//Check if the ray hits a light source.
			auto itr = scene.object_to_light.find(intersection.object);
			if (itr != scene.object_to_light.end())
			{
				if (!light_explicitly_sampled)
				{
					auto le = itr->second->getLe(ray.get_direction(), intersection.plane.normal, intersection.distance);

					return importance < m_rr_threshold ? le * calc_weight : le;
				}
				else
				{
					return glm::vec3(0.0f);
				}
			}

			core::CoordinateSpace tangent_space(intersection.plane.point, intersection.plane.normal, intersection.dpdu);
			auto wi_tangent = tangent_space.vectorToLocalSpace(-ray.get_direction());

			intersection.bsdf_choice = intersection.bsdf_material->chooseBsdf(wi_tangent, uniform_sampler, intersection);

			//DIRECT LIGHTING//
			glm::vec3 direct_lo(0.0f);
			//If the material does not have a delta pdf, then estimate light directly.
			if (!intersection.bsdf_material->hasDeltaDistribution(intersection))
			{
				int size = scene.lights.size();
				for (int i = 0; i < size; ++i)
				{
					const auto* light = scene.lights[i].get();

					auto light_sample = light->sample(uniform_sampler, intersection);
					auto wo_tangent_light = tangent_space.vectorToLocalSpace(light_sample.wo_world);

					auto bsdf = intersection.bsdf_material->getBsdf(wi_tangent, wo_tangent_light, intersection);
					auto cos = glm::abs(core::math::cosTheta(wo_tangent_light));
					auto f = bsdf * light_sample.le * cos / light_sample.pdf_w;

					//One other important thing about this if check is that it never does a computation for NAN values of f.
					glm::vec3 direct_lo_light(0.0f);
					auto f_sum = f.x + f.y + f.z;
					if (f_sum > 0.0f && !std::isinf(f_sum))
					{
						geometry::Ray shadow_ray(intersection.plane.point + light_sample.wo_world * scene.secondary_ray_epsilon, light_sample.wo_world);
						if (!scene.bvh.intersectShadowRay(shadow_ray, light_sample.distance - 1.1f * scene.secondary_ray_epsilon))
						{
							direct_lo_light = f;
						}
					}

					//Apply multiple importance sampling if possible.
					if (intersection.bsdf_material->useMultipleImportanceSampling(intersection) && !light->hasDeltaDistribution())
					{
						//Compute the weight of the sample from light pdf using power heuristic with beta=2
						auto pdf_bsdf = intersection.bsdf_material->getPdf(wi_tangent, wo_tangent_light, intersection);
						auto weight_light = light_sample.pdf_w * light_sample.pdf_w / (light_sample.pdf_w * light_sample.pdf_w + pdf_bsdf * pdf_bsdf);

						if (!std::isnan(weight_light))
						{
							direct_lo_light *= weight_light;
						}

						//Generate a sample according to the bsdf.
						auto w_f = intersection.bsdf_material->sampleWo(wi_tangent, uniform_sampler, intersection);
						const auto& wo_tangent_bsdf = w_f.first;

						//Get the light sample through the sampled direction.
						auto wo_world = tangent_space.vectorToWorldSpace(wo_tangent_bsdf);
						geometry::Ray wo_ray(intersection.plane.point + wo_world * scene.secondary_ray_epsilon, wo_world);
						auto visible_sample = light->getVisibleSample(scene, wo_ray);
						auto f = w_f.second * visible_sample.le;

						//One other important thing about this if check is that it never does a computation for NAN values of f.
						auto f_sum = f.x + f.y + f.z;
						if (f_sum > 0.0f && !std::isinf(f_sum))
						{
							//Compute the weight of the sample from bsdf pdf using power heuristic with beta=2
							auto pdf_bsdf = intersection.bsdf_material->getPdf(wi_tangent, wo_tangent_bsdf, intersection);
							auto weight_bsdf = pdf_bsdf * pdf_bsdf / (visible_sample.pdf_w * visible_sample.pdf_w + pdf_bsdf * pdf_bsdf);

							if (!std::isnan(weight_bsdf))
							{
								direct_lo += f * weight_bsdf;
							}
						}
					}

					direct_lo += direct_lo_light;
				}

				light_explicitly_sampled = true;
			}
			else
			{
				light_explicitly_sampled = false;
			}

			//INDIRECT LIGHTING//
			auto w_f = intersection.bsdf_material->sampleWo(wi_tangent, uniform_sampler, intersection);

			const auto& wo_tangent = w_f.first;
			const auto& f = w_f.second;

			glm::vec3 indirect_lo(0.0f);
			//One other important thing about this if check is that it never does a computation for NAN values of f.
			auto f_sum = f.x + f.y + f.z;
			if (f_sum > 0.0f && !std::isinf(f_sum))
			{
				auto wo_world = tangent_space.vectorToWorldSpace(wo_tangent);
				geometry::Ray wo_ray(intersection.plane.point + wo_world * scene.secondary_ray_epsilon, wo_world);

				indirect_lo = f * estimate(scene, wo_ray, uniform_sampler, importance * glm::min(1.0f, glm::max(glm::max(f.x, f.y), f.z)), light_explicitly_sampled);
			}

			auto lo = direct_lo + indirect_lo;
			return importance < m_rr_threshold ? lo * calc_weight : lo;
		}
	}
}