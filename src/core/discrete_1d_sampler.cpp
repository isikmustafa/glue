#include "discrete_1d_sampler.h"

#include <algorithm>

namespace glue
{
	namespace core
	{
		Discrete1DSampler::Discrete1DSampler(const std::vector<float>& pdf)
		{
			m_cdf.resize(pdf.size());
			std::partial_sum(pdf.begin(), pdf.end(), m_cdf.begin());
			m_sum = m_cdf.back();
		}

		int Discrete1DSampler::sample(UniformSampler& sampler) const
		{
			return static_cast<int>(std::distance(m_cdf.begin(), std::upper_bound(m_cdf.begin(), m_cdf.end(), sampler.sample() * m_sum)));
		}

		float Discrete1DSampler::getPdf(int x) const
		{
			return (x == 0 ? m_cdf[0] : m_cdf[x] - m_cdf[x - 1]) / m_sum;
		}
	}
}