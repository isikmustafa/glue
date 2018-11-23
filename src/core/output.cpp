#include "output.h"
#include "../xml/parser.h"

namespace glue
{
	namespace core
	{
		std::unique_ptr<Output::Xml> Output::Xml::factory(const xml::Node& node)
		{
			auto output_type = node.attribute("type", true);

			if (output_type == std::string("Ldr"))
			{
				return std::make_unique<Ldr::Xml>(node);
			}
			else
			{
				node.throwError("Unknown Output type.");
			}

			return nullptr;
		}

		Ldr::Xml::Xml(const xml::Node& node)
		{
			node.parseChildText("Path", &path);
			node.parseChildText("Format", &format);
			tonemapper = Tonemapper::Xml::factory(node.child("Tonemapper", true));

			if (xml::Parser::gSupportedFormatsSave.find(format) == xml::Parser::gSupportedFormatsSave.end())
			{
				node.child("Format").throwError("Unsupported Format.");
			}
		}

		std::unique_ptr<Output> Ldr::Xml::create() const
		{
			return std::make_unique<Ldr>(*this);
		}

		Ldr::Ldr(const Ldr::Xml& xml)
			: m_path(xml.path + "." + xml.format)
			, m_tonemapper(xml.tonemapper->create())
		{}

		void Ldr::save(const Image& image) const
		{
			m_tonemapper->tonemap(image).saveLdr(m_path);
		}
	}
}