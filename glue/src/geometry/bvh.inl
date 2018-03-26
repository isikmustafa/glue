#include "ray.h"
#include "intersection.h"

#include <algorithm>
#include <array>
#include <thread>

namespace glue
{
	namespace geometry
	{
		template<typename Primitive>
		void buildWithMedianSplitWork(std::vector<Primitive>* objects, std::unique_ptr<BVHNode>* node, int depth)
		{
			auto& ref_node = *node;
			auto& ref_objects = *objects;

			for (int i = ref_node->start; i < ref_node->end; ++i)
			{
				ref_node->bbox.extend(ref_objects[i].getBBox());
			}

			if (ref_node->end - ref_node->start <= 5)
			{
				return;
			}

			auto edges = ref_node->bbox.get_max() - ref_node->bbox.get_min();
			int axis = edges.z > edges.y && edges.z > edges.x ? 2 : (edges.y > edges.x);

			std::nth_element(ref_objects.begin() + ref_node->start, ref_objects.begin() + (ref_node->start + ref_node->end) / 2, ref_objects.begin() + ref_node->end,
				[axis](const Primitive& a, const Primitive& b)
			{
				return a.getBBox().get_max()[axis] + a.getBBox().get_min()[axis] < b.getBBox().get_max()[axis] + b.getBBox().get_min()[axis];
			});

			ref_node->right.reset(new BVHNode((ref_node->start + ref_node->end) / 2, ref_node->end));
			ref_node->left.reset(new BVHNode(ref_node->start, (ref_node->start + ref_node->end) / 2));

			if (depth <= 2)
			{
				std::thread th1(buildWithMedianSplitWork<Primitive>, &ref_objects, &ref_node->right, depth + 1);
				std::thread th2(buildWithMedianSplitWork<Primitive>, &ref_objects, &ref_node->left, depth + 1);
				th2.join();
				th1.join();
			}
			else
			{
				buildWithMedianSplitWork(objects, &ref_node->right, depth + 1);
				buildWithMedianSplitWork(objects, &ref_node->left, depth + 1);
			}
		}

		template<typename Primitive>
		void BVH::buildWithMedianSplit(std::vector<Primitive>& objects)
		{
			m_root.reset(new BVHNode(0, objects.size()));
			buildWithMedianSplitWork(&objects, &m_root, 0);
		}

		template<typename Primitive>
		bool BVH::intersect(const std::vector<Primitive>& objects, const Ray& ray, Intersection& intersection, float max_distance) const
		{
			std::array<BVHNode*, 32> stack;
			int stack_size = 0;
			stack[stack_size++] = m_root.get();
			auto inv_dir = 1.0f / ray.get_direction();

			auto min_distance = max_distance;
			while (stack_size)
			{
				auto top = stack[--stack_size];

				auto result = top->bbox.intersect(ray.get_origin(), inv_dir);
				if (result > 0.0f && result < min_distance)
				{
					if (top->left)
					{
						stack[stack_size++] = top->left.get();
						stack[stack_size++] = top->right.get();
					}
					else
					{
						for (int i = top->start; i < top->end; ++i)
						{
							Intersection temp;
							if (objects[i].intersect(ray, temp, min_distance))
							{
								min_distance = temp.distance;
								intersection = temp;
							}
						}
					}
				}
			}

			return min_distance != max_distance;
		}

		template<typename Primitive>
		bool BVH::intersectShadowRay(const std::vector<Primitive>& objects, const Ray& ray, float max_distance) const
		{
			std::array<BVHNode*, 32> stack;
			int stack_size = 0;
			stack[stack_size++] = m_root.get();
			auto inv_dir = 1.0f / ray.get_direction();

			while (stack_size)
			{
				auto top = stack[--stack_size];

				auto result = top->bbox.intersect(ray.get_origin(), inv_dir);
				if (result > 0.0f && result < max_distance)
				{
					if (top->left)
					{
						stack[stack_size++] = top->left.get();
						stack[stack_size++] = top->right.get();
					}
					else
					{
						for (int i = top->start; i < top->end; ++i)
						{
							if (objects[i].intersectShadowRay(ray, max_distance))
							{
								return true;
							}
						}

					}
				}
			}

			return false;
		}
	}
}