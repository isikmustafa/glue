#ifndef __GLUE__INTEGRATOR__PATHTRACER__
#define __GLUE__INTEGRATOR__PATHTRACER__

#include "integrator.h"
#include "../core/filter.h"
#include "../core/forward_decl.h"

#include <vector>

namespace glue
{
	namespace integrator
	{
		const int cPatchSize = 16;

		class Pathtracer : public Integrator
		{
		public:
			//Xml structure of the class.
			struct Xml : Integrator::Xml
			{
				std::unique_ptr<core::Filter::Xml> filter;
				int sample_count;
				float rr_threshold;

				explicit Xml(const xml::Node& node);
				std::unique_ptr<Integrator> create() const override;
			};

		public:
			explicit Pathtracer(const Pathtracer::Xml& xml);

			void integrate(const core::Scene& scene, core::Image& output) override;

		private:
			std::vector<std::unique_ptr<core::RealSampler>> m_offset_samplers;
			std::vector<core::UniformSampler> m_uniform_samplers;
			std::unique_ptr<core::Filter> m_filter;
			int m_sample_count;
			float m_rr_threshold;

		private:
			void integratePatch(const core::Scene& scene, core::Image& output, int x, int y, int id);
			glm::vec3 estimatePixel(const core::Scene& scene, geometry::Ray& ray, geometry::Intersection& intersection,
				core::UniformSampler& uniform_sampler, float importance, bool light_explicitly_sampled) const;
		};
	}
}

#endif