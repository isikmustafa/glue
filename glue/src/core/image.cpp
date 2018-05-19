#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <glm\geometric.hpp>

namespace glue
{
	namespace core
	{
		Image::Image(int width, int height)
			: m_pixels(width, std::vector<glm::vec3>(height, glm::vec3(0.0f)))
		{}

		std::vector<glm::vec3>& Image::operator[](int x)
		{
			return m_pixels[x];
		}

		const std::vector<glm::vec3>& Image::operator[](int x) const
		{
			return m_pixels[x];
		}

		void Image::save(const std::string& filename) const
		{
			int width = m_pixels.size();
			int height = m_pixels[0].size();
			unsigned char* data = new unsigned char[3 * 3 * m_pixels.size() * m_pixels[0].size()];

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

					int ptr = j * 3 * m_pixels[0].size() + i * 3;
					data[ptr] = static_cast<unsigned char>(pixel.x);
					data[ptr + 1] = static_cast<unsigned char>(pixel.y);
					data[ptr + 2] = static_cast<unsigned char>(pixel.z);
				}
			}

			stbi_write_png(filename.c_str(), m_pixels.size(), m_pixels[0].size(), 3, data, 3 * m_pixels[0].size());
		}

		int Image::width() const
		{
			return m_pixels.size();
		}

		int Image::height() const
		{
			return m_pixels[0].size();
		}
	}
}