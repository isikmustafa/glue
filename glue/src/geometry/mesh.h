#ifndef __GLUE__GEOMETRY__MESH__
#define __GLUE__GEOMETRY__MESH__

#include "object.h"
#include "transformation.h"
#include "bvh.h"
#include "..\core\real_sampler.h"
#include "..\core\discrete_1d_sampler.h"
#include "..\material\bsdf_material.h"

#include <memory>
#include <vector>

namespace glue
{
	namespace geometry
	{
		class Mesh : public Object
		{
		public:
			Mesh(const Transformation& transformation, const BBox& bbox, const std::vector<float>& triangle_areas, float area,
				const std::shared_ptr<BVH<Triangle>>& bvh, std::unique_ptr<material::BsdfMaterial> bsdf_material);

			geometry::Plane samplePlane(core::UniformSampler& sampler) override;
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