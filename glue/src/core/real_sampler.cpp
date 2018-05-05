#include "real_sampler.h"

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

		GaussianSampler::GaussianSampler(float mean, float radius, float sigma)
			: m_generator(std::random_device()())
			, m_distribution(mean, sigma)
			, m_left_bound(mean - radius)
			, m_right_bound(mean + radius)
		{}

		float GaussianSampler::sample()
		{
			auto number = m_distribution(m_generator);
			while (number < m_left_bound || number > m_right_bound)
			{
				number = m_distribution(m_generator);
			}
			return number;
		}
	}
}