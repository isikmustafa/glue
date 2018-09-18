#include "constant_texture.h"
#include "image_texture.h"
#include "checkerboard_2d_texture.h"
#include "checkerboard_3d_texture.h"
#include "perlin_texture.h"
#include "..\xml\node.h"

namespace glue
{
	namespace texture
	{
		std::unique_ptr<Texture::Xml> Texture::Xml::factory(const xml::Node& node)
		{
			auto texture_type = node.attribute("textureType", true);

			if (texture_type == std::string("Constant"))
			{
				return std::make_unique<ConstantTexture::Xml>(node);
			}
			else if (texture_type == std::string("Image"))
			{
				return std::make_unique<ImageTexture::Xml>(node);
			}
			else if (texture_type == std::string("Checkerboard2D"))
			{
				return std::make_unique<Checkerboard2DTexture::Xml>(node);
			}
			else if (texture_type == std::string("Checkerboard3D"))
			{
				return std::make_unique<Checkerboard3DTexture::Xml>(node);
			}
			else if (texture_type == std::string("Perlin"))
			{
				return std::make_unique<PerlinTexture::Xml>(node);
			}
			else
			{
				node.throwError("Unknown Texture type.");
			}
		}
	}
}