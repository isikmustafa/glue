#include "image_texture.h"
#include "../xml/node.h"
#include "../xml/parser.h"

namespace glue
{
	namespace texture
	{
		ImageTexture::Xml::Xml(const xml::Node& node)
		{
			node.parseChildText("Datapath", &datapath);
		}

		std::unique_ptr<Texture> ImageTexture::Xml::create() const
		{
			return std::make_unique<ImageTexture>(*this);
		}

		ImageTexture::ImageTexture(const ImageTexture::Xml& xml)
			: m_images(xml::Parser::loadImage(xml.datapath, false))
		{}

		glm::vec3 ImageTexture::fetch(const geometry::Intersection& intersection) const
		{
			return fetchTexelNearest(intersection.uv, 0);
		}

		glm::vec3 ImageTexture::fetchTexelNearest(const glm::vec2& uv, int mipmap_level) const
		{
			auto& mipmap = (*m_images)[mipmap_level];

			//Always in REPEAT mode.
			auto u = glm::fract(uv.x);
			auto v = 1.0f - glm::fract(uv.y);

			return mipmap.get(static_cast<int>(mipmap.get_width() * u), static_cast<int>(mipmap.get_height() * v));
		}

		int ImageTexture::getWidth() const
		{
			return (*m_images)[0].get_width();
		}

		int ImageTexture::getHeight() const
		{
			return (*m_images)[0].get_height();
		}
	}
}