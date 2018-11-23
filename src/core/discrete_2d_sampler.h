#ifndef __GLUE__CORE__DISCRETE2DSAMPLER__
#define __GLUE__CORE__DISCRETE2DSAMPLER__

#include "discrete_1d_sampler.h"

namespace glue
{
	namespace core
	{
		class Discrete2DSampler
		{
		public:
			Discrete2DSampler() = default;
			explicit Discrete2DSampler(const std::vector<std::vector<float>>& pdfs);

			std::pair<int, int> sample(UniformSampler& sampler) const;
			float getPdf(int x, int y) const;

		private:
			Discrete1DSampler m_col_sampler;
			std::vector<Discrete1DSampler> m_row_samplers;
		};
	}
}

#endif