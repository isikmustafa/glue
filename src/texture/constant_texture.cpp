#include "constant_texture.h"
#include "../xml/node.h"

namespace glue
{
	namespace texture
	{
		ConstantTexture::Xml::Xml(const glm::vec3& p_value)
			: value(p_value)
		{}

		ConstantTexture::Xml::Xml(const xml::Node& node)
		{
			node.parseChildText("Value", &value.x, &value.y, &value.z);
		}

		std::unique_ptr<Texture> ConstantTexture::Xml::create() const
		{
			return std::make_unique<ConstantTexture>(*this);
		}

		ConstantTexture::ConstantTexture(const ConstantTexture::Xml& xml)
			: m_value(xml.value)
		{}

		glm::vec3 ConstantTexture::fetch(const geometry::Intersection& intersection) const
		{
			return m_value;
		}
	}
}