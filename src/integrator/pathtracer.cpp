#include "pathtracer.h"
#include "../geometry/intersection.h"
#include "../core/coordinate_space.h"
#include "../core/scene.h"
#include "../core/math.h"
#include "../material/bsdf_material.h"
#include "../xml/node.h"

#include <omp.h>

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
			: m_uniform_samplers(std::thread::hardware_concurrency())
			, m_filter(xml.filter->create())
			, m_sample_count(xml.sample_count)
			, m_rr_threshold(xml.rr_threshold)
		{
			int numof_cores = std::thread::hardware_concurrency();
			for (int i = 0; i < numof_cores; ++i)
			{
				m_offset_samplers.push_back(m_filter->generateSampler());
			}
		}

		void Pathtracer::integrate(const core::Scene& scene, core::Image& output)
		{
			auto resolution = scene.camera->get_resolution();
			int x, y;

			#pragma omp parallel num_threads(std::thread::hardware_concurrency())
			{
				#pragma omp for schedule(dynamic) collapse(2)
				for (x = 0; x < resolution.x; x += cPTPatchSize)
				{
					for (y = 0; y < resolution.y; y += cPTPatchSize)
					{
						integratePatch(scene, output, x, y, omp_get_thread_num());
					}
				}
			}
		}

		void Pathtracer::integratePatch(const core::Scene& scene, core::Image& output, int x, int y, int id)
		{
			auto resolution = scene.camera->get_resolution();
			auto bound_x = glm::min(cPTPatchSize, resolution.x - x);
			auto bound_y = glm::min(cPTPatchSize, resolution.y - y);

			std::array<std::array<glm::vec3, cPTPatchSize>, cPTPatchSize> final_values;
			std::array<std::array<geometry::Ray, cPTPatchSize>, cPTPatchSize> ray_pool;
			std::array<std::array<geometry::Intersection, cPTPatchSize>, cPTPatchSize> intersection_pool;

			for (int k = 0; k < m_sample_count; ++k)
			{
				for (int i = 0; i < bound_x; ++i)
				{
					for (int j = 0; j < bound_y; ++j)
					{
						ray_pool[i][j] = scene.camera->castRay(x + i, y + j, m_offset_samplers[id]->sample(), m_offset_samplers[id]->sample());
					}
				}

				for (int i = 0; i < bound_x; ++i)
				{
					for (int j = 0; j < bound_y; ++j)
					{
						intersection_pool[i][j] = geometry::Intersection();
						scene.intersect(ray_pool[i][j], intersection_pool[i][j], std::numeric_limits<float>::max());
					}
				}

				auto new_factor = 1.0f / (k + 1);
				auto old_factor = k * new_factor;
				for (int i = 0; i < bound_x; ++i)
				{
					for (int j = 0; j < bound_y; ++j)
					{
						auto& pixel_acc = final_values[i][j];
						pixel_acc *= old_factor;
						pixel_acc += new_factor * estimatePixel(scene, ray_pool[i][j], intersection_pool[i][j], m_uniform_samplers[id], 1.0f, false);
					}
				}
			}

			for (int i = 0; i < bound_x; ++i)
			{
				for (int j = 0; j < bound_y; ++j)
				{
					output.set(x + i, y + j, final_values[i][j]);
				}
			}
		}

		glm::vec3 Pathtracer::estimatePixel(const core::Scene& scene, geometry::Ray& ray, geometry::Intersection& intersection,
			core::UniformSampler& uniform_sampler, float importance, bool light_explicitly_sampled) const
		{
			constexpr float cutoff_probability = 0.5f;
			constexpr float calc_weight = 1.0f / (1.0f - cutoff_probability);

			if (!intersection.object)
			{
				return scene.getBackgroundRadiance(ray.get_direction(), light_explicitly_sampled);
			}

			//Check if the ray hits a light source.
			auto itr = scene.object_to_light.find(intersection.object);
			if (itr != scene.object_to_light.end())
			{
				if (!light_explicitly_sampled)
				{
					return itr->second->getLe(ray.get_direction(), intersection.plane.normal, intersection.distance);
				}
				else
				{
					return glm::vec3(0.0f);
				}
			}

			core::CoordinateSpace tangent_space(intersection.plane.point, intersection.plane.normal, intersection.dpdu);
			auto wo_tangent = tangent_space.vectorToLocalSpace(-ray.get_direction());

			auto chosenbsdf_and_pdf = intersection.bsdf_material->chooseBsdf(wo_tangent, uniform_sampler, intersection);
			intersection.bsdf_choice = chosenbsdf_and_pdf.first;
			auto chosenbsdf_pdf = chosenbsdf_and_pdf.second;

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
					auto wi_tangent_light = tangent_space.vectorToLocalSpace(light_sample.wi_world);

					auto bsdf = intersection.bsdf_material->getBsdf(wi_tangent_light, wo_tangent, intersection);
					auto cos = glm::abs(core::math::cosTheta(wi_tangent_light));
					auto f = bsdf * light_sample.le * cos / light_sample.pdf_w;

					//One other important thing about this if check is that it never does a computation for NAN values of f.
					glm::vec3 direct_lo_light(0.0f);
					auto f_sum = f.x + f.y + f.z;
					if (f_sum > 0.0f && !std::isinf(f_sum))
					{
						geometry::Ray shadow_ray(intersection.plane.point + light_sample.wi_world * scene.secondary_ray_epsilon, light_sample.wi_world);
						if (!scene.intersectShadowRay(shadow_ray, light_sample.distance - 1.1f * scene.secondary_ray_epsilon))
						{
							direct_lo_light = f;
						}
					}

					//Apply multiple importance sampling if possible.
					if (intersection.bsdf_material->useMultipleImportanceSampling(intersection) && !light->hasDeltaDistribution())
					{
						//Compute the weight of the sample from light pdf using power heuristic with beta=2
						auto pdf_bsdf = intersection.bsdf_material->getPdf(wi_tangent_light, wo_tangent, intersection);
						auto weight_light = light_sample.pdf_w * light_sample.pdf_w / (light_sample.pdf_w * light_sample.pdf_w + pdf_bsdf * pdf_bsdf);

						if (!std::isnan(weight_light))
						{
							direct_lo_light *= weight_light;
						}

						//Generate a sample according to the bsdf.
						auto w_f = intersection.bsdf_material->sampleWi(wo_tangent, uniform_sampler, intersection);
						const auto& wi_tangent_bsdf = w_f.first;

						//Get the light sample through the sampled direction.
						auto wi_world = tangent_space.vectorToWorldSpace(wi_tangent_bsdf);
						geometry::Ray wi_ray(intersection.plane.point + wi_world * scene.secondary_ray_epsilon, wi_world);
						auto visible_sample = light->getVisibleSample(scene, wi_ray);
						auto f = w_f.second * visible_sample.le;

						//One other important thing about this if check is that it never does a computation for NAN values of f.
						auto f_sum = f.x + f.y + f.z;
						if (f_sum > 0.0f && !std::isinf(f_sum))
						{
							//Compute the weight of the sample from bsdf pdf using power heuristic with beta=2
							auto pdf_bsdf = intersection.bsdf_material->getPdf(wi_tangent_bsdf, wo_tangent, intersection);
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
			auto w_f = intersection.bsdf_material->sampleWi(wo_tangent, uniform_sampler, intersection);

			const auto& wi_tangent = w_f.first;

			//Account for the probability of bsdf choice.
			auto f = w_f.second / chosenbsdf_pdf;
            direct_lo /= chosenbsdf_pdf;

			glm::vec3 indirect_lo(0.0f);
			//One other important thing about this if check is that it never does a computation for NAN values of f.
			auto f_sum = f.x + f.y + f.z;
			if (f_sum > 0.0f && !std::isinf(f_sum))
			{
				//Russian roulette.
				importance *= glm::min(1.0f, glm::max(glm::max(f.x, f.y), f.z));
				if (importance > m_rr_threshold || uniform_sampler.sample() > cutoff_probability)
				{
					auto wi_world = tangent_space.vectorToWorldSpace(wi_tangent);
					ray = geometry::Ray(intersection.plane.point + wi_world * scene.secondary_ray_epsilon, wi_world);

					intersection = geometry::Intersection();
					scene.intersect(ray, intersection, std::numeric_limits<float>::max());
					indirect_lo = f * estimatePixel(scene, ray, intersection, uniform_sampler, importance, light_explicitly_sampled);
				}
			}

			return direct_lo + (importance < m_rr_threshold ? indirect_lo * calc_weight : indirect_lo);
		}
	}
}