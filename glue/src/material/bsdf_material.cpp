#include "lambertian.h"
#include "oren_nayar.h"
#include "metal.h"
#include "dielectric.h"
#include "..\xml\node.h"

namespace glue
{
	namespace material
	{
		std::unique_ptr<BsdfMaterial::Xml> BsdfMaterial::Xml::factory(const xml::Node& node)
		{
			auto bsdf_type = node.attribute("type", true);

			if (bsdf_type == std::string("Lambertian"))
			{
				return std::make_unique<material::Lambertian::Xml>(node);
			}
			else if (bsdf_type == std::string("OrenNayar"))
			{
				return std::make_unique<material::OrenNayar::Xml>(node);
			}
			else if (bsdf_type == std::string("Metal"))
			{
				return std::make_unique<material::Metal::Xml>(node);
			}
			else if (bsdf_type == std::string("Dielectric"))
			{
				return std::make_unique<material::Dielectric::Xml>(node);
			}
			else
			{
				node.throwError("Unknown BsdfMaterial type.");
			}
		}
	}
}