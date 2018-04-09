#ifndef __GLUE__GEOMETRY__MESH__
#define __GLUE__GEOMETRY__MESH__

#include "bbox.h"
#include "transformation.h"
#include "..\core\uniform_sampler.h"

#include <memory>
#include <vector>

namespace glue
{
	namespace geometry
	{
		class BVH;
		class Triangle;
		class Ray;
		struct Intersection;
		class Mesh
		{
		public:
			Mesh(const Transformation& transformation, const BBox& bbox, std::vector<float> cdf, float area,
				const std::shared_ptr<std::vector<Triangle>>& triangles, const std::shared_ptr<BVH>& bvh);

			glm::vec3 samplePoint(core::UniformSampler& sampler) const;
			float getSurfaceArea() const;
			BBox getBBox() const;
			glm::vec2 getBBoxOnAxis(int axis) const;
			bool intersect(const Ray& ray, Intersection& intersection, float max_distance) const;
			bool intersectShadowRay(const Ray& ray, float max_distance) const;

		private:
			Transformation m_transformation;
			BBox m_bbox;
			std::vector<float> m_cdf; //A CDF to uniformly sample points on the mesh.
			float m_area; //Surface area of the mesh.
			std::shared_ptr<std::vector<Triangle>> m_triangles;
			std::shared_ptr<BVH> m_bvh;
		};
	}
}

#endif