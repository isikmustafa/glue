#ifndef __GLUE__GEOMETRY__OBJECT__
#define __GLUE__GEOMETRY__OBJECT__

#include "bbox.h"
#include "plane.h"
#include "..\core\forward_decl.h"

#include <glm\vec2.hpp>
#include <memory>
#include <unordered_map>

namespace glue
{
	namespace geometry
	{
		class Object
		{
		public:
			//Xml structure of the class.
			struct Xml
			{
				std::unordered_map<std::string, std::string> attributes;

				virtual ~Xml() {}
				virtual std::unique_ptr<Object> create() const = 0;
				static std::unique_ptr<Object::Xml> factory(const xml::Node& node);
			};

		public:
			virtual ~Object() {}

			virtual geometry::Plane samplePlane(core::UniformSampler& sampler) const = 0;
			virtual float getSurfaceArea() const = 0;
			virtual BBox getBBox() const = 0;
			virtual glm::vec2 getBoundsOnAxis(int axis) const = 0;
			virtual bool intersect(const Ray& ray, Intersection& intersection, float max_distance) const = 0;
			virtual bool intersectShadowRay(const Ray& ray, float max_distance) const = 0;
		};
	}
}

#endif