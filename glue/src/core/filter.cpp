#include "filter.h"
#include "real_sampler.h"

namespace glue
{
	namespace core
	{
		std::unique_ptr<RealSampler> BoxFilter::generateSampler() const
		{
			//3x3 window.
			return std::make_unique<UniformSampler>(-1.0f, 2.0f);
		}

		GaussianFilter::GaussianFilter(float sigma)
			: m_sigma(sigma)
		{}

		std::unique_ptr<RealSampler> GaussianFilter::generateSampler() const
		{
			return std::make_unique<GaussianSampler>(0.5f, m_sigma);
		}
	}
}