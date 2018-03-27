#ifndef __GLUE__GEOMETRY__BVH__
#define __GLUE__GEOMETRY__BVH__

#include "bbox.h"

#include <memory>
#include <vector>

namespace glue
{
	namespace geometry
	{
		struct BVHNode
		{
			BBox bbox;
			std::unique_ptr<BVHNode> left;
			std::unique_ptr<BVHNode> right;
			int start;
			int end;

			BVHNode() = default;
			BVHNode(int p_start, int p_end)
				: start(p_start)
				, end(p_end)
			{}
			BVHNode(const BBox& p_bbox, int p_start, int p_end)
				: bbox(p_bbox)
				, start(p_start)
				, end(p_end)
			{}
		};

		class Ray;
		struct Intersection;
		class BVH
		{
		public:
			template<typename Primitive>
			void buildWithMedianSplit(std::vector<Primitive>& objects);

			template<typename Primitive>
			void buildWithSAHSplit(std::vector<Primitive>& objects);

			template<typename Primitive>
			bool intersect(const std::vector<Primitive>& objects, const Ray& ray, Intersection& intersection, float max_distance) const;

			template<typename Primitive>
			bool intersectShadowRay(const std::vector<Primitive>& objects, const Ray& ray, float max_distance) const;

			const std::unique_ptr<BVHNode>& get_root() const { return m_root; }

		private:
			std::unique_ptr<BVHNode> m_root;
		};
	}
}

#include "bvh.inl"

#endif