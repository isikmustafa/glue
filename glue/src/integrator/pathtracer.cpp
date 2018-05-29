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
			if (importance < m_rr_threshold)
			{
				if (uniform_sampler.sample() > 0.5f)
				{
					return glm::vec3(0.0f);
				}
			}

			geometry::Intersection intersection;
			if (!scene.bvh.intersect(ray, intersection, std::numeric_limits<float>::max()))
			{
				return importance < m_rr_threshold ? scene.background_radiance * 2.0f : scene.background_radiance;
			}

			//Check if the ray hits a light source.
			auto itr = scene.object_to_light.find(intersection.object);
			if (itr != scene.object_to_light.end())
			{
				if (!light_explicitly_sampled && glm::dot(-ray.get_direction(), intersection.plane.normal) > 0.0f)
				{
					return importance < m_rr_threshold ? itr->second->getLe() * 2.0f : itr->second->getLe();
				}
				else
				{
					return glm::vec3(0.0f);
				}
			}

			core::CoordinateSpace tangent_space(intersection.plane.point, intersection.plane.normal);
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

					auto sampled_plane = light->samplePlane(uniform_sampler);
					auto wo_world = sampled_plane.point - intersection.plane.point;
					auto distance = glm::length(wo_world);
					wo_world /= distance;

					auto wo_tangent_light = tangent_space.vectorToLocalSpace(wo_world);

					auto bsdf = intersection.bsdf_material->getBsdf(wi_tangent, wo_tangent_light);
					auto cos = glm::abs(core::math::cosTheta(wo_tangent_light));

					//Get p(A) and transform it to p(w)
					auto pdf_light = light->getPdf();
					pdf_light *= distance * distance / glm::max(glm::dot(-wo_world, sampled_plane.normal), 0.0f);

					auto f = bsdf * cos / pdf_light;

					//One other important thing about this if check is that it never does a computation for NAN values of f.
					glm::vec3 direct_lo_light(0.0f);
					if (f.x + f.y + f.z > 0.0f)
					{
						geometry::Ray shadow_ray(intersection.plane.point + wo_world * scene.secondary_ray_epsilon, wo_world);
						if (!scene.bvh.intersectShadowRay(shadow_ray, distance - 1.1f * scene.secondary_ray_epsilon))
						{
							direct_lo_light = f * light->getLe();
						}
					}

					//Apply multiple importance sampling if possible.
					if (intersection.bsdf_material->useMultipleImportanceSampling() && !light->hasDeltaDistribution())
					{
						//Compute the weight of the sample from light pdf using power heuristic with beta=2
						auto pdf_bsdf = intersection.bsdf_material->getPdf(wi_tangent, wo_tangent_light);
						auto weight_light = pdf_light * pdf_light / (pdf_light * pdf_light + pdf_bsdf * pdf_bsdf);

						if (!std::isnan(weight_light))
						{
							direct_lo_light *= weight_light;
						}

						//Generate a sample according to the bsdf.
						auto w_f = intersection.bsdf_material->sampleWo(wi_tangent, uniform_sampler);
						const auto& wo_tangent_bsdf = w_f.first;
						const auto& f = w_f.second;

						//One other important thing about this if check is that it never does a computation for NAN values of f.
						if (f.x + f.y + f.z > 0.0f)
						{
							geometry::Intersection dl_intersection;
							auto wo_world = tangent_space.vectorToWorldSpace(wo_tangent_bsdf);
							geometry::Ray wo_ray(intersection.plane.point + wo_world * scene.secondary_ray_epsilon, wo_world);
							if (scene.bvh.intersect(wo_ray, dl_intersection, std::numeric_limits<float>::max()))
							{
								//If ray through sampled direction hits this light, add its contribution.
								auto itr = scene.object_to_light.find(dl_intersection.object);
								if (itr != scene.object_to_light.end() && itr->second == light)
								{
									//Get p(A) and transform it to p(w)
									auto pdf_light = light->getPdf();
									pdf_light *= dl_intersection.distance * dl_intersection.distance / glm::max(glm::dot(-wo_world, dl_intersection.plane.normal), 0.0f);

									//Compute the weight of the sample from bsdf pdf using power heuristic with beta=2
									auto pdf_bsdf = intersection.bsdf_material->getPdf(wi_tangent, wo_tangent_bsdf);
									auto weight_bsdf = pdf_bsdf * pdf_bsdf / (pdf_light * pdf_light + pdf_bsdf * pdf_bsdf);

									if (!std::isnan(weight_bsdf))
									{
										direct_lo += f * light->getLe() * weight_bsdf;
									}
								}
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
			auto w_f = intersection.bsdf_material->sampleWo(wi_tangent, uniform_sampler);

			const auto& wo_tangent = w_f.first;
			const auto& f = w_f.second;

			glm::vec3 indirect_lo(0.0f);
			//One other important thing about this if check is that it never does a computation for NAN values of f.
			if (f.x + f.y + f.z > 0.0f)
			{
				auto wo_world = tangent_space.vectorToWorldSpace(wo_tangent);
				geometry::Ray wo_ray(intersection.plane.point + wo_world * scene.secondary_ray_epsilon, wo_world);

				indirect_lo = f * estimate(scene, wo_ray, uniform_sampler, importance * glm::max(glm::max(f.x, f.y), f.z), light_explicitly_sampled);
			}

			return importance < m_rr_threshold ? (direct_lo + indirect_lo) * 2.0f : direct_lo + indirect_lo;
		}
	}
}