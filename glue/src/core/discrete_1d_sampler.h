#ifndef __GLUE__CORE__DISCRETE1DSAMPLER__
#define __GLUE__CORE__DISCRETE1DSAMPLER__

#include <random>

namespace glue
{
	namespace core
	{
		class Discrete1DSampler
		{
		public:
			Discrete1DSampler(const std::vector<float>& weights);

			int sample();

		private:
			std::mt19937 m_generator;
			std::discrete_distribution<int> m_distribution;
		};
	}
}

#endif