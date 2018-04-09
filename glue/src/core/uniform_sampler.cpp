#include "uniform_sampler.h"

namespace glue
{
	namespace core
	{
		UniformSampler::UniformSampler(float min, float max)
			: m_generator(std::random_device()())
			, m_distribution(min, max)
		{}

		float UniformSampler::sample()
		{
			return m_distribution(m_generator);
		}
	}
}