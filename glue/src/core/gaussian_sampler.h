#ifndef __GLUE__CORE__GAUSSIANSAMPLER__
#define __GLUE__CORE__GAUSSIANSAMPLER__

#include "sampler.h"

#include <random>

namespace glue
{
	namespace core
	{
		class GaussianSampler : public Sampler
		{
		public:
			//Although no sample will be taken beyond the radius,
			//beware that gaussian function does not integrate to exactly 1.0 between (mean - radius) and (mean + radius).
			//However, the error in the integration is negligible.
			GaussianSampler(float mean, float radius);

			float sample() override;
			
		private:
			std::mt19937 m_generator;
			std::normal_distribution<float> m_distribution;
			float m_left_bound;
			float m_right_bound;
		};
	}
}

#endif