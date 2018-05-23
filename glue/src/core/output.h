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
			virtual ~Output() {}

			virtual void save(const Image& image) const = 0;
		};

		class Ldr : public Output
		{
		public:
			Ldr(const std::string& image_name, std::unique_ptr<Tonemapper> tonemapper);

			void save(const Image& image) const override;

		private:
			std::string m_image_name;
			std::unique_ptr<Tonemapper> m_tonemapper;
		};
	}
}

#endif