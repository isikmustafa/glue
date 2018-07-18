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
		template<typename T, int tMax>
		ImageRepr<T, tMax>::ImageRepr(int width, int height)
			: m_pixels(width, std::vector<RGB>(height))
			, m_width(width)
			, m_height(height)
		{}

		template<typename T, int tMax>
		ImageRepr<T, tMax>::ImageRepr(const std::string& filename)
		{
			//If given image is LDR, stbi_loadf applies an sRGB->Linear conversion.
			int channel;
			auto data = stbi_loadf(filename.c_str(), &m_width, &m_height, &channel, 0);

			if (!data)
			{
				throw std::runtime_error("Error: Image cannot be loaded");
			}

			m_pixels.resize(m_width, std::vector<RGB>(m_height));
			auto index = 0;
			for (int j = 0; j < m_height; ++j)
			{
				for (int i = 0; i < m_width; ++i)
				{
					set(i, j, glm::vec3(data[index], data[index + 1], data[index + 2]));
					index += channel;
				}
			}

			stbi_image_free(data);
		}

		template<typename T, int tMax>
		void ImageRepr<T, tMax>::set(int x, int y, const glm::vec3& value)
		{
			constexpr T max = tMax;
			if constexpr (tMax == 1)
			{
				m_pixels[x][y] = RGB(static_cast<T>(value.x), static_cast<T>(value.y), static_cast<T>(value.z));
			}
			else
			{
				m_pixels[x][y] = RGB(static_cast<T>(value.x * max), static_cast<T>(value.y * max), static_cast<T>(value.z * max));
			}
		}

		template<typename T, int tMax>
		glm::vec3 ImageRepr<T, tMax>::get(int x, int y) const
		{
			if constexpr (tMax == 1)
			{
				const auto& value = m_pixels[x][y];
				return glm::vec3(value.r, value.g, value.b);
			}
			else
			{
				constexpr float inv_max = 1.0f / tMax;
				const auto& value = m_pixels[x][y];
				return glm::vec3(value.r, value.g, value.b) * inv_max;
			}
		}

		Image::Image(int width, int height)
			: m_byte_image(0, 0)
			, m_float_image(width, height)
			, m_width(width)
			, m_height(height)
			, m_type(Type::FLOAT)
		{}

		Image::Image(const std::string& filename)
			: m_byte_image(0, 0)
			, m_float_image(0, 0)
		{
			if (stbi_is_hdr(filename.c_str()))
			{
				m_float_image = FloatImage(filename);
				m_width = m_float_image.get_width();
				m_height = m_float_image.get_height();
				m_type = Type::FLOAT;
			}
			else
			{
				m_byte_image = ByteImage(filename);
				m_width = m_byte_image.get_width();
				m_height = m_byte_image.get_height();
				m_type = Type::BYTE;
			}
		}

		void Image::set(int x, int y, const glm::vec3& value)
		{
			switch (m_type)
			{
			case Type::BYTE:
				m_byte_image.set(x, y, value);
				break;

			case Type::FLOAT:
				m_float_image.set(x, y, value);
				break;
			}
		}

		glm::vec3 Image::get(int x, int y) const
		{
			switch (m_type)
			{
			case Type::BYTE:
				return m_byte_image.get(x, y);
				break;

			case Type::FLOAT:
				return m_float_image.get(x, y);
				break;
			}
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
					auto pixel = get(i, j);

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