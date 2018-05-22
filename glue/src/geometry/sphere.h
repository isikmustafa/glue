#ifndef __GLUE__GEOMETRY__SPHERE__
#define __GLUE__GEOMETRY__SPHERE__

#include "object.h"
#include "transformation.h"
#include "..\material\bsdf_material.h"

#include <memory>
#include <glm\vec3.hpp>

namespace glue
{
	namespace geometry
	{
		class Sphere : public Object
		{
		public:
			Sphere(const Transformation& transformation, const BBox& bbox, float area, std::unique_ptr<material::BsdfMaterial> bsdf_material);

			geometry::Plane samplePlane(core::UniformSampler& sampler) override;
			float getSurfaceArea() const override;
			BBox getBBox() const override;
			glm::vec2 getBoundsOnAxis(int axis) const override;
			bool intersect(const Ray& ray, Intersection& intersection, float max_distance) const override;
			bool intersectShadowRay(const Ray& ray, float max_distance) const override;

		private:
			Transformation m_transformation;
			BBox m_bbox;
			float m_area;
			std::unique_ptr<material::BsdfMaterial> m_bsdf_material;
		};
	}
}

#endif