#ifndef __GLUE__CORE__FILTER__
#define __GLUE__CORE__FILTER__

#include "real_sampler.h"

#include <memory>

namespace glue
{
	namespace core
	{
		class Filter
		{
		public:
			virtual ~Filter() {}

			virtual std::unique_ptr<RealSampler> generateSampler() const = 0;
		};

		class BoxFilter : public Filter
		{
		public:
			std::unique_ptr<RealSampler> generateSampler() const override;
		};

		class GaussianFilter : public Filter
		{
		public:
			GaussianFilter(float sigma);

			std::unique_ptr<RealSampler> generateSampler() const override;

		private:
			float m_sigma;
		};
	}
}

#endif