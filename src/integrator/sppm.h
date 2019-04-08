#ifndef __GLUE__INTEGRATOR__SPPM__
#define __GLUE__INTEGRATOR__SPPM__

#include "integrator.h"
#include "../core/filter.h"
#include "../core/forward_decl.h"
#include "../geometry/plane.h"
#include "../core/coordinate_space.h"

#include <vector>
#include <mutex>
#include <unordered_map>

namespace glue
{
    namespace integrator
    {
        constexpr int cSPPMPatchSize = 16;

        struct GridCell
        {
            int x, y, z;

            GridCell()=default;
            GridCell(int p_x, int p_y, int p_z)
                : x(p_x)
                , y(p_y)
                , z(p_z)
            {}

            bool operator == (const GridCell& gc) const
            {
                return x == gc.x && y == gc.y && z == gc.z;
            }
        };

        struct GridCellHash
        {
            std::size_t operator()(const GridCell& gc) const
            {
                //https://stackoverflow.com/a/17017281/5091304
                std::size_t res = 17;
                res = res * 31 + std::hash<int>()(gc.x);
                res = res * 31 + std::hash<int>()(gc.y);
                res = res * 31 + std::hash<int>()(gc.z);
                return res;
            }
        };

        struct HitPoint
        {
            HitPoint()=default;
            HitPoint(const HitPoint& hp);
            HitPoint& operator = (const HitPoint& hp);

            std::mutex lock;
            core::CoordinateSpace tangent_space;
            glm::vec3 wo_tangent; //Outgoing direction of "rendering equation", that is, inverse of camera ray.
            glm::vec3 direct_lo{ 0.0f, 0.0f, 0.0f };
            glm::vec3 unnormalized_flux{ 0.0f, 0.0f, 0.0f }; //Accumulation of photon contributions. Will be normalized in the end.
            glm::vec3 beta;
            const geometry::Intersection* intersection{ nullptr };
            float acc_count{ 0.0f }; //Total amount of photons contributing to the hitpoint.
            float curr_count{ 0.0f }; //Amount of photons contributing to the hitpoint in the current pass.
            float radius; //Radius of contribution.
        };

        class SPPM : public Integrator
        {
        public:
            //Xml structure of the class.
            struct Xml : Integrator::Xml
            {
                std::unique_ptr<core::Filter::Xml> filter;
                int sample_count;
                int photons_per_pass;
                float rr_threshold;
                float alpha;

                explicit Xml(const xml::Node& node);
                std::unique_ptr<Integrator> create() const override;
            };

        public:
            explicit SPPM(const SPPM::Xml& xml);

            void integrate(const core::Scene& scene, core::Image& output) override;

        private:
            std::vector<std::unordered_map<GridCell, std::vector<HitPoint*>, GridCellHash>> m_grids;
            std::vector<std::vector<geometry::Intersection>> m_intersection_pool;
            std::vector<std::vector<HitPoint>> m_hitpoint_pool;
            std::vector<std::unique_ptr<core::RealSampler>> m_offset_samplers;
            std::vector<core::UniformSampler> m_uniform_samplers;
            std::unique_ptr<core::Filter> m_filter;
            float m_max_search_radius;

            int m_sample_count;
            int m_photons_per_pass;
            float m_rr_threshold;
            float m_alpha;

        private:
            void findHitPoints(const core::Scene& scene, int x, int y, int id);
            void tracePhoton(const core::Scene& scene, int id, float one_over_width);
            float update(const core::Scene& scene, int x, int y);
            void estimateDirect(const core::Scene& scene, geometry::Ray& ray, int x, int y, int id);
        };
    }
}

#endif