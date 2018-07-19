#include "constant_texture.h"
#include "image_texture.h"
#include "..\xml\node.h"

namespace glue
{
	namespace texture
	{
		std::unique_ptr<Texture::Xml> Texture::Xml::factory(const xml::Node& node)
		{
			auto texture_type = node.attribute("type", true);

			if (texture_type == std::string("Constant"))
			{
				return std::make_unique<ConstantTexture::Xml>(node);
			}
			else if (texture_type == std::string("Image"))
			{
				return std::make_unique<ImageTexture::Xml>(node);
			}
			else
			{
				node.throwError("Unknown Texture type.");
			}
		}
	}
}