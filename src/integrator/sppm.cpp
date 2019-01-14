#include "sppm.h"
#include "../geometry/intersection.h"
#include "../core/scene.h"
#include "../core/math.h"
#include "../material/bsdf_material.h"
#include "../xml/node.h"

#include <omp.h>
#include <iostream>

namespace glue
{
    namespace integrator
    {
        HitPoint::HitPoint(const HitPoint& hp)
        {
            *this = hp;
        }

        HitPoint& HitPoint::operator = (const HitPoint& hp)
        {
            tangent_space = hp.tangent_space;
            wo_tangent = hp.wo_tangent;
            direct_lo = hp.direct_lo;
            unnormalized_flux = hp.unnormalized_flux;
            beta = hp.beta;
            intersection = hp.intersection;
            acc_count = hp.acc_count;
            curr_count = hp.curr_count;
            radius = hp.radius;

            return *this;
        }

        SPPM::Xml::Xml(const xml::Node& node)
        {
            filter = core::Filter::Xml::factory(node.child("Filter", true));
            node.parseChildText("SampleCount", &sample_count);
            node.parseChildText("PhotonsPerPass", &photons_per_pass);
            node.parseChildText("RRThreshold", &rr_threshold);
            node.parseChildText("Alpha", &alpha);
        }

        std::unique_ptr<Integrator> SPPM::Xml::create() const
        {
            return std::make_unique<SPPM>(*this);
        }

        SPPM::SPPM(const SPPM::Xml& xml)
            : m_uniform_samplers(std::thread::hardware_concurrency())
            , m_filter(xml.filter->create())
            , m_max_search_radius(0.0f)
            , m_sample_count(xml.sample_count)
            , m_photons_per_pass(xml.photons_per_pass)
            , m_rr_threshold(xml.rr_threshold)
            , m_alpha(xml.alpha)
        {
            int numof_cores = std::thread::hardware_concurrency();
            for (int i = 0; i < numof_cores; ++i)
            {
                m_offset_samplers.push_back(m_filter->generateSampler());
            }
        }

        void SPPM::integrate(const core::Scene& scene, core::Image& output)
        {
            int numof_cores = std::thread::hardware_concurrency();
            auto resolution = scene.camera->get_resolution();
            m_intersection_pool.resize(resolution.x, std::vector<geometry::Intersection>(resolution.y));
            m_hitpoint_pool.resize(resolution.x, std::vector<HitPoint>(resolution.y));

            //Initial estimation for maximum search radius.
            auto scene_bbox = scene.getBBox();
            auto volume_per_pixel = scene_bbox.get_max().x * scene_bbox.get_max().y * scene_bbox.get_max().z / (resolution.x * resolution.y);
            m_max_search_radius = glm::pow(volume_per_pixel, 0.33333f);
            std::cout << m_max_search_radius << std::endl;

            //Initialize hitpoints.
            for (int i = 0; i < resolution.x; ++i)
            {
                for (int j = 0; j < resolution.y; ++j)
                {
                    auto& hitpoint = m_hitpoint_pool[i][j];
                    hitpoint.radius = m_max_search_radius;
                    hitpoint.intersection = &m_intersection_pool[i][j];
                }
            }

            for (int k = 0; k < m_sample_count; ++k)
            {
                m_grids.clear();
                m_grids.resize(numof_cores);

                int x, y, p;
                float new_max_search_radius = 0.0f;
                #pragma omp parallel num_threads(numof_cores)
                {
                    #pragma omp for schedule(dynamic) collapse(2)
                    for (x = 0; x < resolution.x; x += cSPPMPatchSize)
                    {
                        for (y = 0; y < resolution.y; y += cSPPMPatchSize)
                        {
                            findHitPoints(scene, x, y, omp_get_thread_num());
                        }
                    }

                    #pragma omp barrier

                    #pragma omp for schedule(dynamic)
                    for (p = 0; p < m_photons_per_pass; ++p)
                    {
                        tracePhoton(scene, omp_get_thread_num(), 0.5f / m_max_search_radius);
                    }

                    #pragma omp barrier

                    #pragma omp for schedule(dynamic) collapse(2) reduction(max: new_max_search_radius)
                    for (x = 0; x < resolution.x; x += cSPPMPatchSize)
                    {
                        for (y = 0; y < resolution.y; y += cSPPMPatchSize)
                        {
                            auto patch_max_search_radius = update(scene, x, y);
                            new_max_search_radius = glm::max(new_max_search_radius, patch_max_search_radius);
                        }
                    }
                }

                //m_max_search_radius = new_max_search_radius;
                //std::cout << m_max_search_radius << std::endl;
            }

            //Write final values to output.
            for (int i = 0; i < resolution.x; ++i)
            {
                for (int j = 0; j < resolution.y; ++j)
                {
                    auto& hitpoint = m_hitpoint_pool[i][j];

                    auto radiance = hitpoint.direct_lo / static_cast<float>(m_sample_count);
                    radiance += hitpoint.unnormalized_flux /
                            (m_sample_count * m_photons_per_pass * glm::pi<float>() * hitpoint.radius * hitpoint.radius);
                    output.set(i, j, radiance);
                }
            }
        }

        void SPPM::findHitPoints(const core::Scene& scene, int x, int y, int id)
        {
            auto resolution = scene.camera->get_resolution();
            auto bound_x = glm::min(cSPPMPatchSize, resolution.x - x);
            auto bound_y = glm::min(cSPPMPatchSize, resolution.y - y);

            std::array<std::array<geometry::Ray, cSPPMPatchSize>, cSPPMPatchSize> ray_pool;

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
                    m_intersection_pool[x + i][y + j] = geometry::Intersection();
                    scene.intersect(ray_pool[i][j], m_intersection_pool[x + i][y + j], std::numeric_limits<float>::max());
                }
            }

            for (int i = 0; i < bound_x; ++i)
            {
                for (int j = 0; j < bound_y; ++j)
                {
                    estimateDirect(scene, ray_pool[i][j], x + i, y + j, id);
                }
            }
        }

        void SPPM::tracePhoton(const core::Scene& scene, int id, float one_over_width)
        {
            auto& uniform_sampler = m_uniform_samplers[id];
            const auto* light = scene.lights[0].get(); //TODO: Use an index. Not 0.

            auto photon = light->castPhoton(uniform_sampler);
            //This is needed since the origin should be moved a little to avoid self collision.
            photon.ray = geometry::Ray(photon.ray.get_origin() + photon.ray.get_direction() * scene.secondary_ray_epsilon, photon.ray.get_direction());
            
            for (int iteration = 0; ; ++iteration)
            {
                geometry::Intersection intersection;
                intersection.radiance_transport = false;
                auto result = scene.intersect(photon.ray, intersection, std::numeric_limits<float>::max());
                if (!result || !intersection.bsdf_material)
                {
                    break;
                }

                core::CoordinateSpace tangent_space(intersection.plane.point, intersection.plane.normal, intersection.dpdu);
                auto wo_tangent = tangent_space.vectorToLocalSpace(-photon.ray.get_direction());

                //Do not account for direct lighting and contribute to nearby non-specular hitpoints.
                if (iteration > 0 && !intersection.bsdf_material->isSpecular(intersection))
                {
                    GridCell cell(static_cast<int>(std::floor(intersection.plane.point.x * one_over_width)),
                            static_cast<int>(std::floor(intersection.plane.point.y * one_over_width)),
                            static_cast<int>(std::floor(intersection.plane.point.z * one_over_width)));

                    for (auto& grid : m_grids)
                    {
                        auto found = grid.find(cell);
                        if (found != grid.end())
                        {
                            auto& hitpoint_vector = found->second;
                            for (auto hitpoint : hitpoint_vector)
                            {
                                auto diff = hitpoint->intersection->plane.point - intersection.plane.point;
                                if (glm::dot(diff, diff) < hitpoint->radius * hitpoint->radius)
                                {
                                    auto wi_tangent = hitpoint->tangent_space.vectorToLocalSpace(-photon.ray.get_direction());
                                    auto flux_contribution =
                                            photon.beta * hitpoint->beta *
                                            hitpoint->intersection->bsdf_material->getBsdf(wi_tangent, hitpoint->wo_tangent, *hitpoint->intersection);

                                    {
                                        std::unique_lock<std::mutex> ulock(hitpoint->lock);
                                        hitpoint->curr_count += 1.0f;
                                        hitpoint->unnormalized_flux += flux_contribution;
                                    }
                                }
                            }
                        }
                    }
                }

                auto chosenbsdf_and_pdf = intersection.bsdf_material->chooseBsdf(wo_tangent, uniform_sampler, intersection);
                intersection.bsdf_choice = chosenbsdf_and_pdf.first;
                auto chosenbsdf_pdf = chosenbsdf_and_pdf.second;

                auto w_f = intersection.bsdf_material->sampleWi(wo_tangent, uniform_sampler, intersection);
                const auto& wi_tangent = w_f.first;

                //Russian roulette.
                auto q = core::math::rgbToLuminance(photon.beta);
                photon.beta /= chosenbsdf_pdf;
                photon.beta *= w_f.second;
                q = glm::min(1.0f, core::math::rgbToLuminance(photon.beta) / q);
                photon.beta /= q;

                auto beta_sum = photon.beta.x + photon.beta.y + photon.beta.z;
                if (!std::isfinite(beta_sum) || beta_sum <= 0.0f || uniform_sampler.sample() > q)
                {
                    break;
                }

                constexpr float cutoff_probability = 0.5f;
                constexpr float calc_weight = 1.0f / (1.0f - cutoff_probability);
                if (iteration > 8)
                {
                    if (uniform_sampler.sample() < cutoff_probability)
                    {
                        break;
                    }
                    else
                    {
                        photon.beta *= calc_weight;
                    }
                }

                auto wi_world = tangent_space.vectorToWorldSpace(wi_tangent);
                photon.ray = geometry::Ray(intersection.plane.point + wi_world * scene.secondary_ray_epsilon, wi_world);
            }
        }

        float SPPM::update(const core::Scene& scene, int x, int y)
        {
            auto resolution = scene.camera->get_resolution();
            auto bound_x = glm::min(cSPPMPatchSize, resolution.x - x);
            auto bound_y = glm::min(cSPPMPatchSize, resolution.y - y);

            auto max_search_radius = 0.0f;
            for (int i = 0; i < bound_x; ++i)
            {
                for (int j = 0; j < bound_y; ++j)
                {
                    auto& hitpoint = m_hitpoint_pool[x + i][y + j];

                    if (hitpoint.intersection->bsdf_material)
                    {
                        auto new_acc_count = hitpoint.acc_count + m_alpha * hitpoint.curr_count;
                        auto radius_ratios_sqr = new_acc_count / (hitpoint.acc_count + hitpoint.curr_count);
                        hitpoint.acc_count = new_acc_count;

                        if (std::isfinite(radius_ratios_sqr))
                        {
                            hitpoint.radius *= glm::sqrt(radius_ratios_sqr);
                            hitpoint.unnormalized_flux *= radius_ratios_sqr;
                        }

                        max_search_radius = glm::max(max_search_radius, hitpoint.radius);
                    }

                    hitpoint.curr_count = 0.0f;
                }
            }

            return max_search_radius;
        }

        void SPPM::estimateDirect(const core::Scene& scene, geometry::Ray& ray, int x, int y, int id)
        {
            glm::vec3 beta(1.0f);
            bool light_explicitly_sampled = false;
            auto& uniform_sampler = m_uniform_samplers[id];
            auto& intersection = m_intersection_pool[x][y];
            auto& hitpoint = m_hitpoint_pool[x][y];

            while (true)
            {
                if (!intersection.object)
                {
                    hitpoint.direct_lo += beta * scene.getBackgroundRadiance(ray.get_direction(), light_explicitly_sampled);
                    break;
                }

                //Check if the ray hits a light source.
                auto itr = scene.object_to_light.find(intersection.object);
                if (itr != scene.object_to_light.end())
                {
                    if (!light_explicitly_sampled)
                    {
                        hitpoint.direct_lo += beta * itr->second->getLe(ray.get_direction(), intersection.plane.normal, intersection.distance);
                    }
                    break;
                }

                core::CoordinateSpace tangent_space(intersection.plane.point, intersection.plane.normal, intersection.dpdu);
                auto wo_tangent = tangent_space.vectorToLocalSpace(-ray.get_direction());

                auto chosenbsdf_and_pdf = intersection.bsdf_material->chooseBsdf(wo_tangent, uniform_sampler, intersection);
                intersection.bsdf_choice = chosenbsdf_and_pdf.first;
                auto chosenbsdf_pdf = chosenbsdf_and_pdf.second;
                beta /= chosenbsdf_pdf;

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
                        if (std::isfinite(f_sum) && f_sum > 0.0f)
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

                            if (std::isfinite(weight_light))
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
                            if (std::isfinite(f_sum) && f_sum > 0.0f)
                            {
                                //Compute the weight of the sample from bsdf pdf using power heuristic with beta=2
                                auto pdf_bsdf = intersection.bsdf_material->getPdf(wi_tangent_bsdf, wo_tangent, intersection);
                                auto weight_bsdf = pdf_bsdf * pdf_bsdf / (visible_sample.pdf_w * visible_sample.pdf_w + pdf_bsdf * pdf_bsdf);

                                if (std::isfinite(weight_bsdf))
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

                hitpoint.direct_lo += beta * direct_lo;

                //Stop here and add the hitpoint to grid structure to be contributed by photons.
                if (!intersection.bsdf_material->isSpecular(intersection))
                {
                    //Add hitpoint to grid structure.
                    hitpoint.tangent_space = tangent_space;
                    hitpoint.wo_tangent = wo_tangent;
                    hitpoint.beta = beta;

                    const auto& point = intersection.plane.point;
                    auto one_over_width = 0.5f / m_max_search_radius;
                    auto radius_over_width = hitpoint.radius * one_over_width;

                    auto cell_x_center = point.x * one_over_width;
                    auto cell_x_start = static_cast<int>(std::floor(cell_x_center - radius_over_width));
                    auto cell_x_end = static_cast<int>(std::floor(cell_x_center + radius_over_width));

                    auto cell_y_center = point.y * one_over_width;
                    auto cell_y_start = static_cast<int>(std::floor(cell_y_center - radius_over_width));
                    auto cell_y_end = static_cast<int>(std::floor(cell_y_center + radius_over_width));

                    auto cell_z_center = point.z * one_over_width;
                    auto cell_z_start = static_cast<int>(std::floor(cell_z_center - radius_over_width));
                    auto cell_z_end = static_cast<int>(std::floor(cell_z_center + radius_over_width));

                    for (int i = cell_x_start; i <= cell_x_end; ++i)
                    {
                        for (int j = cell_y_start; j <= cell_y_end; ++j)
                        {
                            for (int k = cell_z_start; k <= cell_z_end; ++k)
                            {
                                m_grids[id][GridCell(i, j, k)].push_back(&hitpoint);
                            }
                        }
                    }

                    break;
                }

                //INDIRECT LIGHTING//
                auto w_f = intersection.bsdf_material->sampleWi(wo_tangent, uniform_sampler, intersection);

                const auto& wi_tangent = w_f.first;
                const auto& f = w_f.second;

                auto f_sum = f.x + f.y + f.z;
                if (std::isfinite(f_sum) && f_sum > 0.0f)
                {
                    auto wi_world = tangent_space.vectorToWorldSpace(wi_tangent);
                    ray = geometry::Ray(intersection.plane.point + wi_world * scene.secondary_ray_epsilon, wi_world);

                    intersection = geometry::Intersection();
                    scene.intersect(ray, intersection, std::numeric_limits<float>::max());
                    beta *= f;
                }
                else
                {
                    break;
                }
            }
        }
    }
}