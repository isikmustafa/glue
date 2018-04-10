#ifndef __GLUE__CORE__REALSAMPLER__
#define __GLUE__CORE__REALSAMPLER__

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
	}
}

#endif