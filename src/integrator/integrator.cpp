#include "pathtracer.h"
#include "sppm.h"
#include "../xml/node.h"

namespace glue
{
	namespace integrator
	{
		std::unique_ptr<Integrator::Xml> Integrator::Xml::factory(const xml::Node& node)
		{
			auto integrator_type = node.attribute("type", true);

			if (integrator_type == std::string("Pathtracer"))
			{
				return std::make_unique<Pathtracer::Xml>(node);
			}
			else if (integrator_type == std::string("SPPM"))
            {
                return std::make_unique<SPPM::Xml>(node);
            }
			else
			{
				node.throwError("Unknown Integrator type.");
			}

			return nullptr;
		}
	}
}