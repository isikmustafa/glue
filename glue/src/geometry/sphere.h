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
			//Xml structure of the class.
			struct Xml : public Object::Xml
			{
				float radius;
				glm::vec3 center;
				Transformation::Xml transformation;
				std::unique_ptr<material::BsdfMaterial::Xml> bsdf_material;

				explicit Xml(const xml::Node& node);
				Xml(float p_radius, glm::vec3 p_center, const Transformation::Xml& p_transformation, std::unique_ptr<material::BsdfMaterial::Xml> p_bsdf_material);
				std::unique_ptr<Object> create() const override;
			};

		public:
			Sphere(const Sphere::Xml& xml);

			geometry::Plane samplePlane(core::UniformSampler& sampler) const override;
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