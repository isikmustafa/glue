#ifndef __GLUE__XML__NODE__
#define __GLUE__XML__NODE__

#include <tinyxml2.h>
#include <string>
#include <sstream>
#include <memory>
#include <unordered_map>

namespace glue
{
	namespace xml
	{
		class Node
		{
		public:
			const char* text() const;
			const char* value() const;
			const char* attribute(const std::string& attribute_name, bool throw_if_null = false) const;
			std::unordered_map<std::string, std::string> attributes() const;
			Node child(const std::string& child_name, bool throw_if_null = false) const;
			Node next() const;
			Node parent() const;
			void throwError(const std::string& message) const;
			operator bool() const;
			//Parses child text and assigns them to given arguments.
			template<typename... Args>
			void parseChildText(const std::string& child_name, Args... args) const;

			//Loads the XML file and returns the root of it.
			static Node getRoot(const std::string& xml_filepath);

		private:
			//File will not be closed until the last Node is destroyed.
			std::shared_ptr<tinyxml2::XMLDocument> m_file;
			const tinyxml2::XMLElement* m_node;

		private:
			//Cannot be created outside the class.
			Node(const std::shared_ptr<tinyxml2::XMLDocument>& file, const tinyxml2::XMLElement* node);

			//If default values are NOT provided, these two will be called.
			template<typename T>
			void parse(std::stringstream& stream, T* arg) const;
			template<typename T, typename... Args>
			void parse(std::stringstream& stream, T* arg, Args... args) const;

			//If default values are provided, these two will be called.
			template<typename T>
			void parse(std::stringstream& stream, T* arg, T default_value) const;
			template<typename T, typename... Args>
			void parse(std::stringstream& stream, T* arg, T default_value, Args... args) const;
		};
	}
}

#include "node.inl"

#endif