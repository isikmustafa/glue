#ifndef __GLUE__CORE__DISCRETE1DSAMPLER__
#define __GLUE__CORE__DISCRETE1DSAMPLER__

#include "real_sampler.h"

#include <random>

namespace glue
{
	namespace core
	{
		class Discrete1DSampler
		{
		public:
			Discrete1DSampler() = default;
			Discrete1DSampler(const std::vector<float>& pdf);

			int sample(UniformSampler& sampler) const;

		private:
			std::vector<float> m_cdf;
			float m_sum;
		};
	}
}

#endif