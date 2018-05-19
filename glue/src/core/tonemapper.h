#ifndef __GLUE__CORE__TONEMAPPER__
#define __GLUE__CORE__TONEMAPPER__

#include "image.h"

namespace glue
{
	namespace core
	{
		class Tonemapper
		{
		public:
			virtual ~Tonemapper() {}

			virtual Image tonemap(const Image& image) const = 0;
		};

		//Clamps the pixel values to min and max.
		class Clamp : public Tonemapper
		{
		public:
			Clamp(float min, float max);

			Image tonemap(const Image& image) const override;

		private:
			float m_min;
			float m_max;
		};

		//From Reinhard et al., 2002.
		class GlobalReinhard : public Tonemapper
		{
		public:
			//(maximum luminance of the scene) * max_luminance will be represented as 1.0.
			//e.g. 0.5 means half of (maximum luminance of the scene) will be mapped to 1.0 and the rest will be burnt out.
			GlobalReinhard(float key, float max_luminance);

			Image tonemap(const Image& image) const override;

		private:
			float m_key;
			float m_max_luminance;
		};
	}
}

#endif