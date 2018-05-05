#include "filter.h"
#include "uniform_sampler.h"
#include "gaussian_sampler.h"

namespace glue
{
	namespace core
	{
		std::unique_ptr<RealSampler> BoxFilter::generateSampler() const
		{
			return std::make_unique<UniformSampler>(0.0f, 1.0f);
		}

		GaussianFilter::GaussianFilter(float sigma)
			: m_sigma(sigma)
		{}

		std::unique_ptr<RealSampler> GaussianFilter::generateSampler() const
		{
			return std::make_unique<GaussianSampler>(0.5f, 0.5f, m_sigma);
		}
	}
}