#ifndef __GLUE__CORE__IMAGE__
#define __GLUE__CORE__IMAGE__

#include <glm\vec3.hpp>
#include <vector>
#include <string>
#include <memory>

namespace glue
{
	namespace core
	{
		template<typename T, int tMax>
		class ImageRepr
		{
		public:
			struct RGB
			{
				T r, g, b;

				RGB() = default;
				RGB(T p_r, T p_g, T p_b)
					: r(p_r)
					, g(p_g)
					, b(p_b)
				{}
			};

		public:
			ImageRepr() = default;
			ImageRepr(int width, int height);
			ImageRepr(const std::string& filename);

			void set(int x, int y, const glm::vec3& value);
			glm::vec3 get(int x, int y) const;

		private:
			std::vector<std::vector<RGB>> m_pixels;
		};

		using ByteImage = ImageRepr<unsigned char, 255>;
		using FloatImage = ImageRepr<float, 1>;

		class Image
		{
		public:
			enum class Type
			{
				BYTE,
				FLOAT
			};

		public:
			Image(int width, int height, Type type = Type::FLOAT);
			Image(const std::string& filename);

			void set(int x, int y, const glm::vec3& value);
			glm::vec3 get(int x, int y) const;
			std::vector<Image> generateMipmaps() const;
			void saveLdr(const std::string& filename) const;

			int get_width() const { return m_width; }
			int get_height() const { return m_height; }

		private:
			ByteImage m_byte_image;
			FloatImage m_float_image;
			int m_width;
			int m_height;
			Type m_type;
		};
	}
}

#endif