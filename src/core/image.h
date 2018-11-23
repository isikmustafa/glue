#ifndef __GLUE__CORE__IMAGE__
#define __GLUE__CORE__IMAGE__

#include <glm/vec3.hpp>
#include <vector>
#include <string>
#include <memory>

namespace glue
{
	namespace core
	{
		class ImageReprBase
		{
		public:
			enum class Type
			{
				BYTE,
				FLOAT
			};

		public:
			virtual ~ImageReprBase() = default;

			virtual void set(int x, int y, const glm::vec3& value) = 0;
			virtual glm::vec3 get(int x, int y) const = 0;
			virtual Type type() const = 0;
			virtual std::unique_ptr<ImageReprBase> clone() const = 0;
		};

		template<typename T, int tMax, ImageReprBase::Type tType>
		class ImageRepr : public ImageReprBase
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
			ImageRepr(int width, int height);
			explicit ImageRepr(const std::string& filename);

			void set(int x, int y, const glm::vec3& value) override;
			glm::vec3 get(int x, int y) const override;
			ImageReprBase::Type type() const override;
			std::unique_ptr<ImageReprBase> clone() const override;

		private:
			std::vector<std::vector<RGB>> m_pixels;
		};

		using ByteImage = ImageRepr<unsigned char, 255, ImageReprBase::Type::BYTE>;
		using FloatImage = ImageRepr<float, 1, ImageReprBase::Type::FLOAT>;

		class Image
		{
		public:
			Image(int width, int height, ImageReprBase::Type type = ImageReprBase::Type::FLOAT);
			explicit Image(const std::string& filename);
			Image(const Image& image);
			Image(Image&& image) = default;
			Image& operator=(const Image& image);
			Image& operator=(Image&& image) = default;

			void set(int x, int y, const glm::vec3& value);
			glm::vec3 get(int x, int y) const;
			std::vector<Image> generateMipmaps() const;
			void saveLdr(const std::string& filename) const;

			int get_width() const { return m_width; }
			int get_height() const { return m_height; }

		private:
			std::unique_ptr<ImageReprBase> m_image_repr;
			int m_width;
			int m_height;
		};
	}
}

#endif