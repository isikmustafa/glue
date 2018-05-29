#ifndef __GLUE__CORE__OUTPUT__
#define __GLUE__CORE__OUTPUT__

#include "image.h"
#include "tonemapper.h"

#include <memory>

namespace glue
{
	namespace core
	{
		class Output
		{
		public:
			//Xml structure of the class.
			struct Xml
			{
				virtual ~Xml() {}
				virtual std::unique_ptr<Output> create() const = 0;
				static std::unique_ptr<Output::Xml> factory(const xml::Node& node);
			};

		public:
			virtual ~Output() {}

			virtual void save(const Image& image) const = 0;
		};

		class Ldr : public Output
		{
		public:
			//Xml structure of the class.
			struct Xml : public Output::Xml
			{
				std::string path;
				std::string format;
				std::unique_ptr<Tonemapper::Xml> tonemapper;

				explicit Xml(const xml::Node& node);
				std::unique_ptr<Output> create() const override;
			};

		public:
			Ldr(const Ldr::Xml& xml);

			void save(const Image& image) const override;

		private:
			std::string m_path;
			std::unique_ptr<Tonemapper> m_tonemapper;
		};
	}
}

#endif