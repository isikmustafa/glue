#include "output.h"
namespace glue
{
	namespace core
	{
		Ldr::Ldr(const std::string& image_name, std::unique_ptr<Tonemapper> tonemapper)
			: m_image_name(image_name)
			, m_tonemapper(std::move(tonemapper))
		{}

		void Ldr::save(const Image& image) const
		{
			m_tonemapper->tonemap(image).saveLdr(m_image_name);
		}
	}
}