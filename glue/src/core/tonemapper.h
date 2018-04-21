#ifndef __GLUE__CORE__TONEMAPPER__
#define __GLUE__CORE__TONEMAPPER__

#include "hdr_image.h"

namespace glue
{
	namespace core
	{
		class Tonemapper
		{
		public:
			virtual ~Tonemapper() {}

			virtual HdrImage tonemap(const HdrImage& hdr_image) const = 0;
		};

		//Clamps the pixel values to min and max.
		class Clamp : public Tonemapper
		{
		public:
			Clamp(float min, float max);

			HdrImage tonemap(const HdrImage& hdr_image) const override;

		private:
			float m_min;
			float m_max;
		};

		//From Reinhard et al., 2002.
		class GlobalReinhard : public Tonemapper
		{
		public:
			GlobalReinhard(float key);

			HdrImage tonemap(const HdrImage& hdr_image) const override;

		private:
			float m_key;
		};
	}
}

#endif