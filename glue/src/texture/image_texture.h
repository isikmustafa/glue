#ifndef __GLUE__TEXTURE__IMAGETEXTURE__
#define __GLUE__TEXTURE__IMAGETEXTURE__

#include "texture.h"
#include "..\core\image.h"

#include <memory>

namespace glue
{
	namespace texture
	{
		class ImageTexture : public Texture
		{
		public:
			enum class Interpolation
			{
				NEAREST,
				BILINEAR
			};

		public:
			//Xml structure of the class.
			struct Xml : public Texture::Xml
			{
				std::string datapath;
				Interpolation texel_interpolation;
				Interpolation mipmap_interpolation;

				explicit Xml(const xml::Node& node);
				std::unique_ptr<Texture> create() const override;
			};

		public:
			ImageTexture(const ImageTexture::Xml& xml);

			glm::vec3 fetch(const geometry::Intersection& intersection) const override;

		private:
			std::shared_ptr<std::vector<core::Image>> m_images;
			Interpolation m_texel_interpolation;
			Interpolation m_mipmap_interpolation;
		};
	}
}

#endif