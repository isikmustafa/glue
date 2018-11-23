#ifndef __GLUE__TEXTURE__PERLINTEXTURE__
#define __GLUE__TEXTURE__PERLINTEXTURE__

#include "texture.h"

namespace glue
{
	namespace texture
	{
		class PerlinTexture : public Texture
		{
		public:
			//Xml structure of the class.
			struct Xml : public Texture::Xml
			{
				glm::vec3 first;
				glm::vec3 second;
				float frequency;

				explicit Xml(const xml::Node& node);
				std::unique_ptr<Texture> create() const override;
			};

		public:
			explicit PerlinTexture(const PerlinTexture::Xml& xml);

			glm::vec3 fetch(const geometry::Intersection& intersection) const override;

		private:
			glm::vec3 m_first;
			glm::vec3 m_second;
			float m_frequency;
		};
	}
}

#endif