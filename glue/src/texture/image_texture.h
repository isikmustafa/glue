#ifndef __GLUE__TEXTURE__IMAGETEXTURE__
#define __GLUE__TEXTURE__IMAGETEXTURE__

#include "texture.h"
#include "..\core\image.h"

#include <memory>

namespace glue
{
	namespace texture
	{
		class ImageTexture : public Texture
		{
		public:
			//Xml structure of the class.
			struct Xml : public Texture::Xml
			{
				std::string datapath;

				explicit Xml(const xml::Node& node);
				std::unique_ptr<Texture> create() const override;
			};

		public:
			ImageTexture(const ImageTexture::Xml& xml);

			glm::vec3 fetch(const geometry::Intersection& intersection) const override;
			glm::vec3 fetchTexelNearest(const glm::vec2& uv, int mipmap_level) const;
			int getWidth() const;
			int getHeight() const;

		private:
			std::shared_ptr<std::vector<core::Image>> m_images;
			float m_coeff_u;
			float m_coeff_v;
		};
	}
}

#endif