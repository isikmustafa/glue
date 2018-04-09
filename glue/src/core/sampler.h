#ifndef __GLUE__CORE__SAMPLER__
#define __GLUE__CORE__SAMPLER__

namespace glue
{
	namespace core
	{
		class Sampler
		{
		public:
			virtual ~Sampler() {}

			virtual float sample() = 0;
		};
	}
}

#endif