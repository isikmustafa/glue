#ifndef __GLUE__INTEGRATOR__PATHTRACER__
#define __GLUE__INTEGRATOR__PATHTRACER__

#include "integrator.h"
#include "..\core\filter.h"
#include "..\core\forward_decl.h"

namespace glue
{
	namespace integrator
	{
		class Pathtracer : public Integrator
		{
		public:
			Pathtracer(std::unique_ptr<core::Filter> filter, int sample_count);

			glm::vec3 integratePixel(const core::Scene& scene, int x, int y) const override;

		private:
			std::unique_ptr<core::Filter> m_filter;
			int m_sample_count;

		private:
			glm::vec3 estimate(const core::Scene& scene, const geometry::Ray& ray,
				core::UniformSampler& uniform_sampler, float importance, bool light_explicitly_sampled) const;
		};
	}
}

#endif