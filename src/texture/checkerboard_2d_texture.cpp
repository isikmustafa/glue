#include "checkerboard_2d_texture.h"
#include "../xml/node.h"

#include <glm/geometric.hpp>

namespace glue
{
	namespace texture
	{
		Checkerboard2DTexture::Xml::Xml(const xml::Node& node)
		{
			node.parseChildText("First", &first.x, &first.y, &first.z);
			node.parseChildText("Second", &second.x, &second.y, &second.z);
			node.parseChildText("Frequency", &frequency);
		}

		std::unique_ptr<Texture> Checkerboard2DTexture::Xml::create() const
		{
			return std::make_unique<Checkerboard2DTexture>(*this);
		}

		Checkerboard2DTexture::Checkerboard2DTexture(const Checkerboard2DTexture::Xml& xml)
			: m_first(xml.first)
			, m_second(xml.second)
			, m_frequency(xml.frequency)
		{}

		glm::vec3 Checkerboard2DTexture::fetch(const geometry::Intersection& intersection) const
		{
			auto uv = intersection.uv * m_frequency;
			auto first_or_second = static_cast<bool>((static_cast<int>(glm::floor(uv.x) + glm::floor(uv.y))) % 2);

			return first_or_second ? m_first : m_second;
		}
	}
}