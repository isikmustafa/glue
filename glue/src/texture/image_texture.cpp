#include "image_texture.h"
#include "..\xml\node.h"
#include "..\xml\parser.h"
#include "..\core\math.h"

#include <glm\geometric.hpp>

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
			: m_images(xml::Parser::loadImage(xml.datapath))
		{
			auto ratio = static_cast<float>((*m_images)[0].get_width()) / (*m_images)[0].get_height();
			m_coeff_u = (ratio > 1.0f ? ratio : 1.0f);
			m_coeff_v = (ratio < 1.0f ? 1.0f / ratio : 1.0f);
		}

		glm::vec3 ImageTexture::fetch(const geometry::Intersection& intersection) const
		{
			auto max_u = glm::max(intersection.duvdx.x, intersection.duvdy.x) * m_coeff_u;
			auto max_v = glm::max(intersection.duvdx.y, intersection.duvdy.y) * m_coeff_v;

			auto width = glm::max(max_u, max_v);
			auto mipmap_level = m_images->size() - 2 + glm::log2(width);

			if (mipmap_level < 0.0f)
			{
				return fetchTexel(intersection.uv, 0);
			}
			else if (mipmap_level >= m_images->size() - 1.0f)
			{
				return fetchTexel(intersection.uv, m_images->size() - 1);
			}
			else
			{
				//Always blend between mipmap levels with a triangle filter (linear interpolation).
				int floor_level = static_cast<int>(glm::floor(mipmap_level));
				return core::math::lerp(mipmap_level - floor_level, fetchTexel(intersection.uv, floor_level), fetchTexel(intersection.uv, floor_level + 1));
			}
		}

		glm::vec3 ImageTexture::fetchTexel(const glm::vec2& uv, int mipmap_level) const
		{
			auto& mipmap = (*m_images)[mipmap_level];

			//Always in REPEAT-NEAREST mode.
			auto u = glm::fract(uv.x);
			auto v = glm::fract(uv.y);

			return mipmap.get(static_cast<int>(mipmap.get_width() * u), static_cast<int>(mipmap.get_height() * v));
		}
	}
}