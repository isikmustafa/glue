#include "hdr_image.h"

#include <png.hpp>
#include <glm\geometric.hpp>

namespace glue
{
	namespace core
	{
		HdrImage::HdrImage(int width, int height)
			: m_pixels(width, std::vector<glm::vec3>(height, glm::vec3(0.0f)))
		{}

		std::vector<glm::vec3>& HdrImage::operator[](int x)
		{
			return m_pixels[x];
		}

		const std::vector<glm::vec3>& HdrImage::operator[](int x) const
		{
			return m_pixels[x];
		}

		void HdrImage::save(const std::string& filename) const
		{
			int width = m_pixels.size();
			int height = m_pixels[0].size();
			png::image<png::rgb_pixel> image(width, height);

			for (int i = 0; i < width; ++i)
			{
				for (int j = 0; j < height; ++j)
				{
					auto pixel = m_pixels[i][j];

					//Gamma correction, 1/2.2
					pixel.x = glm::pow(pixel.x, 0.454545f);
					pixel.y = glm::pow(pixel.y, 0.454545f);
					pixel.z = glm::pow(pixel.z, 0.454545f);

					pixel *= 255.0f;
					image[j][i] = png::rgb_pixel(static_cast<png::byte>(pixel.x), static_cast<png::byte>(pixel.y), static_cast<png::byte>(pixel.z));
				}
			}

			image.write(filename.c_str());
		}

		int HdrImage::width() const
		{
			return m_pixels.size();
		}

		int HdrImage::height() const
		{
			return m_pixels[0].size();
		}
	}
}