#include "tonemapper.h"
#include "..\xml\node.h"

#include <glm\geometric.hpp>
#include "output.h"

namespace glue
{
	namespace core
	{
		std::unique_ptr<Tonemapper::Xml> Tonemapper::Xml::factory(const xml::Node& node)
		{
			auto tonemapper_type = node.attribute("type", true);

			if (tonemapper_type == std::string("Clamp"))
			{
				return std::make_unique<Clamp::Xml>(node);
			}
			else if (tonemapper_type == std::string("GlobalReinhard"))
			{
				return std::make_unique<GlobalReinhard::Xml>(node);
			}
			else
			{
				node.throwError("Unknown Tonemapper type.");
			}
		}

		Clamp::Xml::Xml(const xml::Node& node)
		{
			node.parseChildText("Min", &min);
			node.parseChildText("Max", &max);
		}

		std::unique_ptr<Tonemapper> Clamp::Xml::create() const
		{
			return std::make_unique<Clamp>(*this);
		}

		Clamp::Clamp(const Clamp::Xml& xml)
			: m_min(xml.min)
			, m_max(xml.max)
		{}

		Image Clamp::tonemap(const Image& image) const
		{
			auto tonemapped_image = image;
			int width = tonemapped_image.get_width();
			int height = tonemapped_image.get_height();

			for (int i = 0; i < width; ++i)
			{
				for (int j = 0; j < height; ++j)
				{
					auto pixel = tonemapped_image.get(i, j);
					tonemapped_image.set(i, j, glm::clamp(pixel, m_min, m_max));
				}
			}

			return tonemapped_image;
		}

		GlobalReinhard::Xml::Xml(const xml::Node& node)
		{
			node.parseChildText("Key", &key);
			node.parseChildText("MaxLuminance", &max_luminance);
		}

		std::unique_ptr<Tonemapper> GlobalReinhard::Xml::create() const
		{
			return std::make_unique<GlobalReinhard>(*this);
		}

		GlobalReinhard::GlobalReinhard(const GlobalReinhard::Xml& xml)
			: m_key(xml.key)
			, m_max_luminance(xml.max_luminance)
		{}

		Image GlobalReinhard::tonemap(const Image& image) const
		{
			auto tonemapped_image = image;
			int width = tonemapped_image.get_width();
			int height = tonemapped_image.get_height();

			constexpr auto epsilon = 0.0001f;
			auto geometric_average = 0.0f;
			auto max_l_t = 0.0f;
			for (int i = 0; i < width; ++i)
			{
				for (int j = 0; j < height; ++j)
				{
					auto pixel = tonemapped_image.get(i, j);

					auto luminance = glm::dot(glm::vec3(0.2126f, 0.7152f, 0.0722f), pixel);
					max_l_t = glm::max(luminance, max_l_t);

					geometric_average += glm::log(epsilon + luminance);
				}
			}
			geometric_average = glm::exp(geometric_average / (width * height));
			auto scale = m_key / geometric_average;

			max_l_t *= (scale * m_max_luminance);

			for (int i = 0; i < width; ++i)
			{
				for (int j = 0; j < height; ++j)
				{
					auto pixel = tonemapped_image.get(i, j);

					auto l_t = scale * glm::dot(glm::vec3(0.2126f, 0.7152f, 0.0722f), pixel);

					pixel *= scale * (1.0f + l_t / (max_l_t * max_l_t)) / (1.0f + l_t);
					tonemapped_image.set(i, j, glm::clamp(pixel, 0.0f, 1.0f));
				}
			}

			return tonemapped_image;
		}
	}
}