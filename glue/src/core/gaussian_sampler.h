#ifndef __GLUE__CORE__GAUSSIANSAMPLER__
#define __GLUE__CORE__GAUSSIANSAMPLER__

#include "real_sampler.h"

#include <random>

namespace glue
{
	namespace core
	{
		class GaussianSampler : public RealSampler
		{
		public:
			GaussianSampler(float mean = 0.5f, float radius = 0.5f, float sigma = 0.5f);

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