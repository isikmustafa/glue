#ifndef __GLUE__GEOMETRY__OBJECT__
#define __GLUE__GEOMETRY__OBJECT__

#include "bbox.h"
#include "plane.h"
#include "..\core\forward_decl.h"

#include <glm\vec2.hpp>

namespace glue
{
	namespace geometry
	{
		//This is a base class for Meshes and Spheres.
		//Although defined as virtual, intersect and intersectShadowRay functions are not dispatched in runtime.
		//Since BVH that uses them is implemented as a template, these functions are dispatched in compile time.
		//They are defined as virtual only to have a common structure.
		class Object
		{
		public:
			virtual ~Object() {}

			virtual geometry::Plane samplePlane(core::UniformSampler& sampler) = 0;
			virtual float getSurfaceArea() const = 0;
			virtual BBox getBBox() const = 0;
			virtual glm::vec2 getBoundsOnAxis(int axis) const = 0;
			virtual bool intersect(const Ray& ray, Intersection& intersection, float max_distance) const = 0;
			virtual bool intersectShadowRay(const Ray& ray, float max_distance) const = 0;
		};
	}
}

#endif