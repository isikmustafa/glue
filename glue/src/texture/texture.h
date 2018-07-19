#ifndef __GLUE__TEXTURE__TEXTURE__
#define __GLUE__TEXTURE__TEXTURE__

#include "..\geometry\intersection.h"

#include <memory>

namespace glue
{
	namespace texture
	{
		class Texture
		{
		public:
			//Xml structure of the class.
			struct Xml
			{
				virtual ~Xml() {}
				virtual std::unique_ptr<Texture> create() const = 0;
				static std::unique_ptr<Texture::Xml> factory(const xml::Node& node);
			};

		public:
			virtual ~Texture() {}

			virtual glm::vec3 fetch(const geometry::Intersection& intersection) const = 0;
		};
	}
}

#endif