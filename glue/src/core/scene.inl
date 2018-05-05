#include <ctpl_stl.h>
#include <thread>

namespace glue
{
	namespace core
	{
		template<typename T>
		void Scene::parseTag(T* arg)
		{
			if (!(m_stream >> (*arg)))
			{
				throw std::runtime_error("Error: Tag content is not found!");
			}
		}

		template<typename T, typename... Args>
		void Scene::parseTag(T* arg, Args... args)
		{
			if (!(m_stream >> (*arg)))
			{
				throw std::runtime_error("Error: Tag content is not found!");
			}
			parseTag(args...);
		}

		template<typename T>
		void Scene::parseTag(T* arg, T default_value)
		{
			if (!(m_stream >> (*arg)))
			{
				*arg = default_value;
			}
		}

		template<typename T, typename... Args>
		void Scene::parseTag(T* arg, T default_value, Args... args)
		{
			if (!(m_stream >> (*arg)))
			{
				*arg = default_value;
			}
			parseTag(args...);
		}

		template<typename... Args>
		void Scene::parseTagContent(tinyxml2::XMLElement* element, Args... args)
		{
			if (element)
			{
				m_stream << element->GetText() << std::endl;
			}
			parseTag(args...);

			m_stream.clear();
		}

		template<typename T>
		tinyxml2::XMLElement* Scene::getFirstChildElementThrow(T element, const std::string& tag_name)
		{
			auto child = element->FirstChildElement(tag_name.c_str());

			if (!child)
			{
				throw std::runtime_error("Error: " + tag_name + " of " + element->Value() + " is not found!");
			}

			return child;
		}

		template<typename T>
		const char* Scene::getAttributeThrow(T element, const std::string& att_name)
		{
			auto att = element->Attribute(att_name.c_str());

			if (!att)
			{
				throw std::runtime_error("Error: " + std::string(att_name) + " of " + element->Value() + " is not found!");
			}

			return att;
		}
	}
}