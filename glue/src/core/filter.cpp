#include "filter.h"
#include "real_sampler.h"

namespace glue
{
	namespace core
	{
		std::unique_ptr<RealSampler> BoxFilter::generateSampler() const
		{
			return std::make_unique<UniformSampler>(-0.5f, 1.5f);
		}

		std::unique_ptr<RealSampler> TentFilter::generateSampler() const
		{
			return std::make_unique<TentSampler>(0.5f, 1.0f);
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