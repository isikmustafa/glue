#ifndef __GLUE__LIGHT__LIGHT__
#define __GLUE__LIGHT__LIGHT__

#include "..\geometry\plane.h"
#include "..\geometry\object.h"
#include "..\core\forward_decl.h"

#include <glm\vec3.hpp>
#include <memory>

namespace glue
{
	namespace light
	{
		//This class is not designed to abstract every possible light.
		//Its functions and function signatures may change over time.
		class Light
		{
		public:
			//Xml structure of the class.
			struct Xml
			{
				virtual ~Xml() {}
				virtual std::unique_ptr<Light> create() const = 0;
				static std::unique_ptr<Light::Xml> factory(const xml::Node& node);
			};

		public:
			virtual ~Light() {}

			virtual geometry::Plane samplePlane(core::UniformSampler& sampler) const = 0;
			virtual glm::vec3 getLe() const = 0;
			virtual float getPdf() const = 0;
			virtual bool hasDeltaDistribution() const = 0;
			virtual std::shared_ptr<geometry::Object> getObject() const = 0;
		};
	}
}

#endif