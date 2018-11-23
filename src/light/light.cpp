#include "diffuse_arealight.h"
#include "environment_light.h"
#include "pointlight.h"
#include "../xml/node.h"

namespace glue
{
	namespace light
	{
		std::unique_ptr<Light::Xml> Light::Xml::factory(const xml::Node& node)
		{
			auto light_type = node.attribute("type", true);

			if (light_type == std::string("DiffuseArealight"))
			{
				return std::make_unique<DiffuseArealight::Xml>(node);
			}
			else if (light_type == std::string("EnvironmentLight"))
			{
				return std::make_unique<EnvironmentLight::Xml>(node);
			}
			else if (light_type == std::string("Pointlight"))
			{
				return std::make_unique<Pointlight::Xml>(node);
			}
			else
			{
				node.throwError("Unknown Light type.");
			}

			return nullptr;
		}
	}
}