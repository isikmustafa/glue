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

		GlobalReinhard::GlobalReinhard(float key, float max_luminance)
			: m_key(key)
			, m_max_luminance(max_luminance)
		{}

		HdrImage GlobalReinhard::tonemap(const HdrImage& hdr_image) const
		{
			auto tonemapped_image = hdr_image;
			int width = tonemapped_image.width();
			int height = tonemapped_image.height();

			constexpr auto epsilon = 0.0001f;
			auto geometric_average = 0.0f;
			auto max_l_t = 0.0f;
			for (int i = 0; i < width; ++i)
			{
				for (int j = 0; j < height; ++j)
				{
					const auto& pixel = tonemapped_image[i][j];

					auto luminance = glm::dot(glm::vec3(0.2126f, 0.7152f, 0.0722f), pixel);
					max_l_t = glm::max(luminance, max_l_t);

					geometric_average += glm::log(epsilon + luminance);
				}
			}
			geometric_average = glm::exp(geometric_average / (width * height));
			auto scale = m_key / geometric_average;

			max_l_t *= (scale * m_max_luminance);

			for (int i = 0; i < width; ++i)
			{
				for (int j = 0; j < height; ++j)
				{
					auto& pixel = tonemapped_image[i][j];

					auto l_t = scale * glm::dot(glm::vec3(0.2126f, 0.7152f, 0.0722f), pixel);

					pixel *= scale * (1.0f + l_t / (max_l_t * max_l_t)) / (1.0f + l_t);
					pixel = glm::clamp(pixel, 0.0f, 1.0f);
				}
			}

			return tonemapped_image;
		}
	}
}