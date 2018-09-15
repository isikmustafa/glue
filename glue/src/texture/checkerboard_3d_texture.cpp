#include "checkerboard_3d_texture.h"
#include "..\xml\node.h"

#include <glm\geometric.hpp>

namespace glue
{
	namespace texture
	{
		Checkerboard3DTexture::Xml::Xml(const xml::Node& node)
		{
			node.parseChildText("First", &first.x, &first.y, &first.z);
			node.parseChildText("Second", &second.x, &second.y, &second.z);
			node.parseChildText("Frequency", &frequency);
		}

		std::unique_ptr<Texture> Checkerboard3DTexture::Xml::create() const
		{
			return std::make_unique<Checkerboard3DTexture>(*this);
		}

		Checkerboard3DTexture::Checkerboard3DTexture(const Checkerboard3DTexture::Xml& xml)
			: m_first(xml.first)
			, m_second(xml.second)
			, m_frequency(xml.frequency)
		{}

		glm::vec3 Checkerboard3DTexture::fetch(const geometry::Intersection& intersection) const
		{
			auto p = intersection.plane.point * m_frequency;
			bool first_or_second = (static_cast<int>(glm::floor(p.x) + glm::floor(p.y) + glm::floor(p.z))) % 2;

			return first_or_second ? m_first : m_second;
		}
	}
}