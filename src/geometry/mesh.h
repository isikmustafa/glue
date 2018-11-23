#ifndef __GLUE__GEOMETRY__MESH__
#define __GLUE__GEOMETRY__MESH__

#include "object.h"
#include "transformation.h"
#include "bvh.h"
#include "../core/real_sampler.h"
#include "../core/discrete_1d_sampler.h"
#include "../material/bsdf_material.h"

#include <memory>
#include <vector>

namespace glue
{
	namespace geometry
	{
		class Mesh : public Object
		{
		public:
			//Xml structure of the class.
			struct Xml : public Object::Xml
			{
				std::string datapath;
				Transformation::Xml transformation;
				std::unique_ptr<material::BsdfMaterial::Xml> bsdf_material;

				explicit Xml(const xml::Node& node);
				Xml(const std::string& p_datapath, const Transformation::Xml& p_transformation, std::unique_ptr<material::BsdfMaterial::Xml> p_bsdf_material);
				std::unique_ptr<Object> create() const override;
			};

		public:
			explicit Mesh(const Mesh::Xml& xml);

			geometry::Plane samplePlane(core::UniformSampler& sampler) const override;
			float getSurfaceArea() const override;
			BBox getBBox() const override;
			glm::vec2 getBoundsOnAxis(int axis) const override;
			bool intersect(const Ray& ray, Intersection& intersection, float max_distance) const override;
			bool intersectShadowRay(const Ray& ray, float max_distance) const override;

		private:
			Transformation m_transformation;
			BBox m_bbox;
			core::Discrete1DSampler m_triangle_sampler;
			float m_area;
			std::shared_ptr<BVH<Triangle>> m_bvh;
			std::unique_ptr<material::BsdfMaterial> m_bsdf_material;
		};
	}
}

#endif