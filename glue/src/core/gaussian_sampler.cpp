#include "gaussian_sampler.h"

namespace glue
{
	namespace core
	{
		GaussianSampler::GaussianSampler(float mean, float radius)
			: m_generator(std::random_device()())
			, m_distribution(mean, radius / 4.0f)
			, m_left_bound(mean - radius)
			, m_right_bound(mean + radius)
		{}

		float GaussianSampler::sample()
		{
			auto number = m_distribution(m_generator);
			while (number < m_left_bound || number >= m_right_bound)
			{
				number = m_distribution(m_generator);
			}
			return number;
		}
	}
}