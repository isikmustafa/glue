#include "discrete_1d_sampler.h"

namespace glue
{
	namespace core
	{
		Discrete1DSampler::Discrete1DSampler(const std::vector<float>& weights)
			: m_generator(std::random_device()())
			, m_distribution(weights.begin(), weights.end())
		{}

		int Discrete1DSampler::sample()
		{
			return m_distribution(m_generator);
		}
	}
}