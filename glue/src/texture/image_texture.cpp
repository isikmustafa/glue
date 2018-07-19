#include "image_texture.h"
#include "..\xml\node.h"
#include "..\xml\parser.h"

#include <glm\geometric.hpp>

namespace glue
{
	namespace texture
	{
		ImageTexture::Xml::Xml(const xml::Node& node)
		{
			node.parseChildText("Datapath", &datapath);
			std::string texel_interpolation_str;
			node.parseChildText("TexelInterpolation", &texel_interpolation_str);
			std::string mipmap_interpolation_str;
			node.parseChildText("MipmapInterpolation", &mipmap_interpolation_str);

			if (texel_interpolation_str == "Nearest")
			{
				texel_interpolation = Interpolation::NEAREST;
			}
			else if (texel_interpolation_str == "Bilinear")
			{
				texel_interpolation = Interpolation::BILINEAR;
			}
			else
			{
				node.throwError("Unknown TexelInterpolation value.");
			}

			if (mipmap_interpolation_str == "Nearest")
			{
				mipmap_interpolation = Interpolation::NEAREST;
			}
			else if (mipmap_interpolation_str == "Bilinear")
			{
				mipmap_interpolation = Interpolation::BILINEAR;
			}
			else
			{
				node.throwError("Unknown MipmapInterpolation value.");
			}
		}

		std::unique_ptr<Texture> ImageTexture::Xml::create() const
		{
			return std::make_unique<ImageTexture>(*this);
		}

		ImageTexture::ImageTexture(const ImageTexture::Xml& xml)
			: m_images(xml::Parser::loadImage(xml.datapath))
			, m_texel_interpolation(xml.texel_interpolation)
			, m_mipmap_interpolation(xml.mipmap_interpolation)
		{}

		glm::vec3 ImageTexture::fetch(const geometry::Intersection& intersection) const
		{
			//Always clamp for now.
			//Nearest-Nearest
			auto& mipmap = (*m_images)[0];
			float u = glm::clamp(intersection.uv.x, 0.0f, 0.9999f);
			float v = glm::clamp(intersection.uv.y, 0.0f, 0.9999f);

			return mipmap.get(static_cast<int>(mipmap.get_width() * u), static_cast<int>(mipmap.get_height() * v));
		}
	}
}