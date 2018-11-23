#include "real_sampler.h"

#include <glm/geometric.hpp>

namespace glue
{
	namespace core
	{
		UniformSampler::UniformSampler(float min, float max)
			: m_generator(std::random_device()())
			, m_distribution(min, max)
		{}

		float UniformSampler::sample()
		{
			return m_distribution(m_generator);
		}

		TentSampler::TentSampler(float center, float radius)
			: m_sampler(-1.0f, 1.0f)
			, m_center(center)
			, m_radius(radius)
		{}

		float TentSampler::sample()
		{
			auto sample = m_sampler.sample();
			if (sample > 0.0f)
			{
				return (1.0f - glm::sqrt(1.0f - sample)) * m_radius + m_center;
			}
			else
			{
				return (-1.0f + glm::sqrt(1.0f + sample)) * m_radius + m_center;
			}
		}

		GaussianSampler::GaussianSampler(float mean, float sigma)
			: m_generator(std::random_device()())
			, m_distribution(mean, sigma)
		{}

		float GaussianSampler::sample()
		{
			return m_distribution(m_generator);
		}
	}
}