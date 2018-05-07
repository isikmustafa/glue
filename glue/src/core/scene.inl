namespace glue
{
	namespace core
	{
		template<typename T>
		void Scene::parseTag(T* arg)
		{
			if (!(m_stream >> (*arg)))
			{
				throw std::runtime_error("");
			}
		}

		template<typename T, typename... Args>
		void Scene::parseTag(T* arg, Args... args)
		{
			if (!(m_stream >> (*arg)))
			{
				throw std::runtime_error("");
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
		void Scene::parseTagContent(tinyxml2::XMLElement* element, const std::string& tag, Args... args)
		{
			auto child = element->FirstChildElement(tag.c_str());
			if (child)
			{
				m_stream << child->GetText() << std::endl;
			}

			try
			{
				parseTag(args...);
			}
			catch (const std::runtime_error&)
			{
				auto line = std::to_string(element->GetLineNum());
				throw std::runtime_error("Error near line " + line + ": " + tag + " of " + element->Value() + " is not specified.");
			}

			m_stream.clear();
		}

		template<typename T>
		tinyxml2::XMLElement* Scene::getFirstChildElementThrow(T element, const std::string& tag_name)
		{
			auto child = element->FirstChildElement(tag_name.c_str());

			if (!child)
			{
				auto line = std::to_string(element->GetLineNum());
				throw std::runtime_error("Error near line " + line + ": " + tag_name + " of " + element->Value() + " is not specified.");
			}

			return child;
		}

		template<typename T>
		const char* Scene::getAttributeThrow(T element, const std::string& att_name)
		{
			auto att = element->Attribute(att_name.c_str());

			if (!att)
			{
				auto line = std::to_string(element->GetLineNum());
				throw std::runtime_error("Error near line " + line + ": " + att_name + " of " + element->Value() + " is not specified.");
			}

			return att;
		}
	}
}