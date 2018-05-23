#include "image.h"

#include <memory>
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
			: m_pixels(width, std::vector<glm::vec3>(height))
			, m_width(width)
			, m_height(height)
		{}

		Image::Image(const std::string& filename)
		{
			//If given image is LDR, stbi_loadf applies an sRGB->Linear conversion.
			int channel;
			auto data = stbi_loadf(filename.c_str(), &m_width, &m_height, &channel, 0);

			if (!data)
			{
				throw std::runtime_error("Error: Image cannot be loaded");
			}

			m_pixels.resize(m_width, std::vector<glm::vec3>(m_height));
			auto index = 0;
			for (int j = 0; j < m_height; ++j)
			{
				for (int i = 0; i < m_width; ++i)
				{
					m_pixels[i][j] = glm::vec3(data[index], data[index + 1], data[index + 2]);
					index += channel;
				}
			}

			stbi_image_free(data);
		}

		std::vector<glm::vec3>& Image::operator[](int i)
		{
			return m_pixels[i];
		}

		const std::vector<glm::vec3>& Image::operator[](int i) const
		{
			return m_pixels[i];
		}

		void Image::saveLdr(const std::string& filename) const
		{
			constexpr int channel = 3; //RGB
			int stride = channel * m_width;
			std::unique_ptr<unsigned char> data(new unsigned char[stride * m_height]);
			auto data_ptr = data.get();

			auto index = 0;
			for (int j = 0; j < m_height; ++j)
			{
				for (int i = 0; i < m_width; ++i)
				{
					auto pixel = m_pixels[i][j];

					//Linear->sRGB
					pixel.x = glm::pow(pixel.x, 0.45454545f);
					pixel.y = glm::pow(pixel.y, 0.45454545f);
					pixel.z = glm::pow(pixel.z, 0.45454545f);

					pixel *= 255.0f;

					data_ptr[index] = static_cast<unsigned char>(pixel.x);
					data_ptr[index + 1] = static_cast<unsigned char>(pixel.y);
					data_ptr[index + 2] = static_cast<unsigned char>(pixel.z);
					index += channel;
				}
			}

			auto image_format = filename.substr(filename.find_last_of('.') + 1);

			if (image_format == "png")
			{
				stbi_write_png(filename.c_str(), m_width, m_height, channel, data_ptr, stride);
			}
			else if (image_format == "bmp")
			{
				stbi_write_bmp(filename.c_str(), m_width, m_height, channel, data_ptr);
			}
			else if (image_format == "tga")
			{
				stbi_write_tga(filename.c_str(), m_width, m_height, channel, data_ptr);
			}
		}
	}
}