#ifndef __GLUE__CORE__REALSAMPLER__
#define __GLUE__CORE__REALSAMPLER__

#include <random>

namespace glue
{
	namespace core
	{
		class RealSampler
		{
		public:
			virtual ~RealSampler() {}

			virtual float sample() = 0;
		};

		class UniformSampler : public RealSampler
		{
		public:
			UniformSampler(float min = 0.0f, float max = 1.0f);

			float sample() override;

		private:
			std::mt19937 m_generator;
			std::uniform_real_distribution<float> m_distribution;
		};

		class GaussianSampler : public RealSampler
		{
		public:
			GaussianSampler(float mean = 0.5f, float radius = 0.5f, float sigma = 0.5f);

			float sample() override;

		private:
			std::mt19937 m_generator;
			std::normal_distribution<float> m_distribution;
			float m_left_bound;
			float m_right_bound;
		};
	}
}

#endif