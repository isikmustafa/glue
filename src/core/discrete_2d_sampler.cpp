#include "discrete_2d_sampler.h"

namespace glue
{
	namespace core
	{
		Discrete2DSampler::Discrete2DSampler(const std::vector<std::vector<float>>& pdfs)
		{
			std::vector<float> col_sums;

			for (const auto& pdf : pdfs)
			{
				m_row_samplers.emplace_back(pdf);
				col_sums.push_back(m_row_samplers.back().get_sum());
			}

			m_col_sampler = Discrete1DSampler(col_sums);
		}

		std::pair<int, int> Discrete2DSampler::sample(UniformSampler& sampler) const
		{
			auto col = m_col_sampler.sample(sampler);

			return std::make_pair(col, m_row_samplers[col].sample(sampler));
		}

		float Discrete2DSampler::getPdf(int x, int y) const
		{
			return m_col_sampler.getPdf(x) * m_row_samplers[x].getPdf(y);
		}
	}
}