namespace glue
{
	namespace xml
	{
		template<typename T>
		void Node::parse(std::stringstream& stream, T* arg) const
		{
			if (!(stream >> (*arg)))
			{
				throw std::runtime_error("");
			}
		}

		template<typename T, typename... Args>
		void Node::parse(std::stringstream& stream, T* arg, Args... args) const
		{
			if (!(stream >> (*arg)))
			{
				throw std::runtime_error("");
			}
			parse(stream, args...);
		}

		template<typename T>
		void Node::parse(std::stringstream& stream, T* arg, T default_value) const
		{
			if (!(stream >> (*arg)))
			{
				*arg = default_value;
			}
		}

		template<typename T, typename... Args>
		void Node::parse(std::stringstream& stream, T* arg, T default_value, Args... args) const
		{
			if (!(stream >> (*arg)))
			{
				*arg = default_value;
			}
			parse(stream, args...);
		}

		template<typename... Args>
		void Node::parseChildText(const std::string& child_name, Args... args) const
		{
			std::stringstream stream;
			auto child = this->child(child_name);
			if (child)
			{
				stream << child.text() << std::endl;
			}

			try
			{
				parse(stream, args...);
			}
			catch (const std::runtime_error&)
			{
				throwError(child_name + " of " + value() + " is not specified.");
			}
		}
	}
}