#ifndef __GLUE__INTEGRATOR__INTEGRATOR__
#define __GLUE__INTEGRATOR__INTEGRATOR__

#include "..\core\forward_decl.h"

#include <glm\vec3.hpp>
#include <memory>

namespace glue
{
	namespace integrator
	{
		class Integrator
		{
		public:
			//Xml structure of the class.
			struct Xml
			{
				virtual ~Xml() {}
				virtual std::unique_ptr<Integrator> create() const = 0;
				static std::unique_ptr<Integrator::Xml> factory(const xml::Node& node);
			};

		public:
			virtual ~Integrator() {}

			virtual glm::vec3 integratePixel(const core::Scene& scene, int x, int y) const = 0;
		};
	}
}

#endif