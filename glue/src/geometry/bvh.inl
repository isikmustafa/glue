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

			if (ref_node->end - ref_node->start <= 1)
			{
				return;
			}

			auto edges = ref_node->bbox.get_max() - ref_node->bbox.get_min();
			int axis = edges.z > edges.y && edges.z > edges.x ? 2 : (edges.y > edges.x);

			std::nth_element(ref_objects.begin() + ref_node->start, ref_objects.begin() + (ref_node->start + ref_node->end) / 2, ref_objects.begin() + ref_node->end,
				[axis](const Primitive& a, const Primitive& b)
			{
				auto a_bbox = a.getBBoxOnAxis(axis);
				auto b_bbox = b.getBBoxOnAxis(axis);
				return a_bbox.x + a_bbox.y < b_bbox.x + b_bbox.y;
			});

			ref_node->right.reset(new BVHNode((ref_node->start + ref_node->end) / 2, ref_node->end));
			ref_node->left.reset(new BVHNode(ref_node->start, (ref_node->start + ref_node->end) / 2));

			if (depth <= 2)
			{
				std::thread th1(buildWithMedianSplitWork<Primitive>, objects, &ref_node->right, depth + 1);
				std::thread th2(buildWithMedianSplitWork<Primitive>, objects, &ref_node->left, depth + 1);
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
		void buildWithSAHSplitWork(std::vector<Primitive>* objects, std::unique_ptr<BVHNode>* node, float work)
		{
			auto& ref_node = *node;
			auto& ref_objects = *objects;

			if (ref_node->end - ref_node->start <= 5)
			{
				return;
			}

			auto min_cost = std::numeric_limits<float>::max();
			int right_count;
			BBox left_bbox, right_bbox;
			int cut_axis;
			for (int axis = 0; axis < 3; ++axis)
			{
				//Fill the bins and count the objects inside them.
				constexpr int cBinSize = 128;
				BBox bins[cBinSize];
				int counts[cBinSize] = { 0 };
				auto constant_term = cBinSize * (1 - std::numeric_limits<float>::epsilon()) / (ref_node->bbox.get_max()[axis] - ref_node->bbox.get_min()[axis]);
				for (int i = ref_node->start; i < ref_node->end; ++i)
				{
					auto object_bbox = ref_objects[i].getBBox();
					auto bin_index = static_cast<int>(((object_bbox.get_min()[axis] + object_bbox.get_max()[axis]) * 0.5f - ref_node->bbox.get_min()[axis]) * constant_term);
					bins[bin_index].extend(object_bbox);
					++counts[bin_index];
				}

				//Compute all the cumulative bboxes by sweeping from left to right.
				BBox cbins[cBinSize];
				cbins[0] = bins[0];
				for (int i = 1; i < cBinSize; ++i)
				{
					cbins[i] = cbins[i - 1];
					cbins[i].extend(bins[i]);
				}

				//Compute all the possible costs by sweeping from right to left and find the optimal one.
				BBox crightbin;
				int crightcount = 0;
				for (int i = cBinSize - 1; i >= 1; --i)
				{
					crightbin.extend(bins[i]);
					crightcount += counts[i];

					auto left_count = ref_node->end - ref_node->start - crightcount;
					auto cost = cbins[i - 1].getSurfaceArea() * left_count + crightbin.getSurfaceArea() * crightcount;
					if (cost < min_cost && left_count && crightcount)
					{
						min_cost = cost;
						right_count = crightcount;
						left_bbox = cbins[i - 1];
						right_bbox = crightbin;
						cut_axis = axis;
					}
				}
			}

			std::nth_element(ref_objects.begin() + ref_node->start, ref_objects.begin() + (ref_node->end - right_count), ref_objects.begin() + ref_node->end,
				[cut_axis](const Primitive& a, const Primitive& b)
			{
				auto a_bbox = a.getBBoxOnAxis(cut_axis);
				auto b_bbox = b.getBBoxOnAxis(cut_axis);
				return a_bbox.x + a_bbox.y < b_bbox.x + b_bbox.y;
			});

			ref_node->right.reset(new BVHNode(right_bbox, ref_node->end - right_count, ref_node->end));
			ref_node->left.reset(new BVHNode(left_bbox, ref_node->start, ref_node->end - right_count));

			if (work > 1.5f)
			{
				auto right_work = work * (static_cast<float>(right_count) / (ref_node->end - ref_node->start));
				std::thread th1(buildWithSAHSplitWork<Primitive>, objects, &ref_node->right, right_work);
				std::thread th2(buildWithSAHSplitWork<Primitive>, objects, &ref_node->left, work - right_work);
				th2.join();
				th1.join();
			}
			else
			{
				buildWithSAHSplitWork(objects, &ref_node->right, 0.0f);
				buildWithSAHSplitWork(objects, &ref_node->left, 0.0f);
			}
		}

		template<typename Primitive>
		void BVH::buildWithSAHSplit(std::vector<Primitive>& objects)
		{
			int size = objects.size();
			BBox temp;
			for (int i = 0; i < size; ++i)
			{
				temp.extend(objects[i].getBBox());
			}
			m_root.reset(new BVHNode(temp, 0, size));
			buildWithSAHSplitWork(&objects, &m_root, 32.0f);
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
				if (result.x > 0.0f && result.y < min_distance)
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
				if (result.x > 0.0f && result.y < max_distance)
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