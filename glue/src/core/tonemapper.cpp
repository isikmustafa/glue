#include "tonemapper.h"

#include <glm\geometric.hpp>

namespace glue
{
	namespace core
	{
		Clamp::Clamp(float min, float max)
			: m_min(min)
			, m_max(max)
		{}

		HdrImage Clamp::tonemap(const HdrImage& hdr_image) const
		{
			auto tonemapped_image = hdr_image;
			int width = tonemapped_image.width();
			int height = tonemapped_image.height();

			for (int i = 0; i < width; ++i)
			{
				for (int j = 0; j < height; ++j)
				{
					auto& pixel = tonemapped_image[i][j];
					pixel = glm::clamp(pixel, m_min, m_max);
				}
			}

			return tonemapped_image;
		}

		GlobalReinhard::GlobalReinhard(float key)
			: m_key(key)
		{}

		HdrImage GlobalReinhard::tonemap(const HdrImage& hdr_image) const
		{
			auto tonemapped_image = hdr_image;
			int width = tonemapped_image.width();
			int height = tonemapped_image.height();

			constexpr auto epsilon = 0.0001f;
			auto geometric_average = 0.0f;
			for (int i = 0; i < width; ++i)
			{
				for (int j = 0; j < height; ++j)
				{
					const auto& pixel = tonemapped_image[i][j];
					geometric_average += glm::log(epsilon + (pixel.x + pixel.y + pixel.z) * 0.3333f);
				}
			}
			geometric_average /= (width * height);
			geometric_average = glm::exp(geometric_average);
			geometric_average /= m_key;

			for (int i = 0; i < width; ++i)
			{
				for (int j = 0; j < height; ++j)
				{
					auto& pixel = tonemapped_image[i][j];
					pixel.x /= (pixel.x + geometric_average);
					pixel.y /= (pixel.y + geometric_average);
					pixel.z /= (pixel.z + geometric_average);
				}
			}

			return tonemapped_image;
		}
	}
}