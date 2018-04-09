#ifndef __GLUE__CORE__UNIFORMSAMPLER__
#define __GLUE__CORE__UNIFORMSAMPLER__

#include "sampler.h"

#include <random>

namespace glue
{
	namespace core
	{
		class UniformSampler : public Sampler
		{
		public:
			UniformSampler(float min, float max);

			float sample() override;

		private:
			std::mt19937 m_generator;
			std::uniform_real_distribution<float> m_distribution;
		};
	}
}

#endif