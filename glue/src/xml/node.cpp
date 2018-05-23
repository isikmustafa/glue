#include "node.h"

namespace glue
{
	namespace xml
	{
		Node::Node(const std::shared_ptr<tinyxml2::XMLDocument>& file, const tinyxml2::XMLElement* node)
			: m_file(file)
			, m_node(node)
		{}

		const char* Node::text() const
		{
			return m_node->GetText();
		}

		const char* Node::value() const
		{
			return m_node->Value();
		}

		const char* Node::attribute(const std::string& attribute_name, bool throw_if_null) const
		{
			auto attribute = m_node->Attribute(attribute_name.c_str());
			if (throw_if_null && !attribute)
			{
				throwError(attribute_name + " of " + value() + " is not specified.");
			}

			return attribute;
		}

		Node Node::child(const std::string& child_name, bool throw_if_null) const
		{
			Node child(m_file, m_node->FirstChildElement(child_name.c_str()));
			if (throw_if_null && !child)
			{
				throwError(child_name + " of " + value() + " is not specified.");
			}

			return child;
		}

		Node Node::next() const
		{
			return Node(m_file, m_node->NextSiblingElement(value()));
		}

		Node Node::parent() const
		{
			return Node(m_file, m_node->Parent()->ToElement());
		}

		void Node::throwError(const std::string& message) const
		{
			auto line = std::to_string(m_node->GetLineNum());
			throw std::runtime_error("Error near line " + line + ": " + message);
		}

		Node::operator bool() const
		{
			return m_node != nullptr;
		}

		Node Node::getRoot(const std::string& xml_filepath)
		{
			auto file = std::make_shared<tinyxml2::XMLDocument>();
			if (file->LoadFile(xml_filepath.c_str()))
			{
				throw std::runtime_error(file->ErrorName());
			}

			return Node(file, file->RootElement());
		}
	}
}