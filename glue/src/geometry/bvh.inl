#include "ray.h"
#include "intersection.h"

#include <algorithm>
#include <array>
#include <thread>
#include <type_traits>

//A trick to check if the type has operator ->
//Slightly modified version of https://stackoverflow.com/a/15394527/5091304
template<typename T, typename = void>
struct isDereferenceable : std::is_function<T> {};

template<typename T>
struct isDereferenceable<T, typename std::enable_if<
	std::is_same<decltype(void(&T::operator->)), void>::value
>::type> : std::true_type {};

namespace glue
{
	namespace geometry
	{
		template<typename Primitive>
		void BVH<Primitive>::addObject(Primitive primitive)
		{
			m_objects.push_back(std::move(primitive));
		}

		template<typename Primitive>
		void BVH<Primitive>::buildWithMedianSplit()
		{
			m_root = std::make_unique<BVHNode>(0, m_objects.size());
			buildWithMedianSplitWork(&m_root, std::thread::hardware_concurrency());
		}

		template<typename Primitive>
		void BVH<Primitive>::buildWithSAHSplit()
		{
			int size = m_objects.size();
			BBox temp;
			for (int i = 0; i < size; ++i)
			{
				if constexpr (isDereferenceable<Primitive>::value)
				{
					temp.extend(m_objects[i]->getBBox());
				}
				else
				{
					temp.extend(m_objects[i].getBBox());
				}
			}
			m_root = std::make_unique<BVHNode>(temp, 0, size);
			buildWithSAHSplitWork(&m_root, static_cast<float>(std::thread::hardware_concurrency()));
		}

		template<typename Primitive>
		void BVH<Primitive>::buildWithMedianSplitWork(std::unique_ptr<BVHNode>* node, int work)
		{
			auto& ref_node = *node;

			for (int i = ref_node->start; i < ref_node->end; ++i)
			{
				if constexpr (isDereferenceable<Primitive>::value)
				{
					ref_node->bbox.extend(m_objects[i]->getBBox());
				}
				else
				{
					ref_node->bbox.extend(m_objects[i].getBBox());
				}
			}

			if (ref_node->end - ref_node->start <= 1)
			{
				return;
			}

			auto edges = ref_node->bbox.get_max() - ref_node->bbox.get_min();
			int axis = edges.z > edges.y && edges.z > edges.x ? 2 : (edges.y > edges.x);

			if constexpr (isDereferenceable<Primitive>::value)
			{
				std::nth_element(m_objects.begin() + ref_node->start, m_objects.begin() + (ref_node->start + ref_node->end) / 2, m_objects.begin() + ref_node->end,
					[axis](const Primitive& a, const Primitive& b)
				{
					auto a_bbox = a->getBoundsOnAxis(axis);
					auto b_bbox = b->getBoundsOnAxis(axis);
					return a_bbox.x + a_bbox.y < b_bbox.x + b_bbox.y;
				});
			}
			else
			{
				std::nth_element(m_objects.begin() + ref_node->start, m_objects.begin() + (ref_node->start + ref_node->end) / 2, m_objects.begin() + ref_node->end,
					[axis](const Primitive& a, const Primitive& b)
				{
					auto a_bbox = a.getBoundsOnAxis(axis);
					auto b_bbox = b.getBoundsOnAxis(axis);
					return a_bbox.x + a_bbox.y < b_bbox.x + b_bbox.y;
				});
			}

			ref_node->right = std::make_unique<BVHNode>((ref_node->start + ref_node->end) / 2, ref_node->end);
			ref_node->left = std::make_unique<BVHNode>(ref_node->start, (ref_node->start + ref_node->end) / 2);

			if (work > 1)
			{
				std::thread th1(&BVH<Primitive>::buildWithMedianSplitWork, this, &ref_node->right, work >> 1);
				std::thread th2(&BVH<Primitive>::buildWithMedianSplitWork, this, &ref_node->left, work >> 1);
				th2.join();
				th1.join();
			}
			else
			{
				buildWithMedianSplitWork(&ref_node->right, 0);
				buildWithMedianSplitWork(&ref_node->left, 0);
			}
		}

		template<typename Primitive>
		void BVH<Primitive>::buildWithSAHSplitWork(std::unique_ptr<BVHNode>* node, float work)
		{
			auto& ref_node = *node;

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
					BBox object_bbox;
					if constexpr (isDereferenceable<Primitive>::value)
					{
						object_bbox = m_objects[i]->getBBox();
					}
					else
					{
						object_bbox = m_objects[i].getBBox();
					}
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

			if constexpr (isDereferenceable<Primitive>::value)
			{
				std::nth_element(m_objects.begin() + ref_node->start, m_objects.begin() + (ref_node->end - right_count), m_objects.begin() + ref_node->end,
					[cut_axis](const Primitive& a, const Primitive& b)
				{
					auto a_bbox = a->getBoundsOnAxis(cut_axis);
					auto b_bbox = b->getBoundsOnAxis(cut_axis);
					return a_bbox.x + a_bbox.y < b_bbox.x + b_bbox.y;
				});
			}
			else
			{
				std::nth_element(m_objects.begin() + ref_node->start, m_objects.begin() + (ref_node->end - right_count), m_objects.begin() + ref_node->end,
					[cut_axis](const Primitive& a, const Primitive& b)
				{
					auto a_bbox = a.getBoundsOnAxis(cut_axis);
					auto b_bbox = b.getBoundsOnAxis(cut_axis);
					return a_bbox.x + a_bbox.y < b_bbox.x + b_bbox.y;
				});
			}

			ref_node->right = std::make_unique<BVHNode>(right_bbox, ref_node->end - right_count, ref_node->end);
			ref_node->left = std::make_unique<BVHNode>(left_bbox, ref_node->start, ref_node->end - right_count);

			if (work > 0.5f)
			{
				auto right_work = work * (static_cast<float>(right_count) / (ref_node->end - ref_node->start));
				std::thread th1(&BVH<Primitive>::buildWithSAHSplitWork, this, &ref_node->right, right_work);
				std::thread th2(&BVH<Primitive>::buildWithSAHSplitWork, this, &ref_node->left, work - right_work);
				th2.join();
				th1.join();
			}
			else
			{
				buildWithSAHSplitWork(&ref_node->right, 0.0f);
				buildWithSAHSplitWork(&ref_node->left, 0.0f);
			}
		}

		template<typename Primitive>
		bool BVH<Primitive>::intersect(const Ray& ray, Intersection& intersection, float max_distance) const
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

						//If min of left bbox is closer to ray, process left node first in the next iteration.
						if (glm::dot(top->left->bbox.get_min() - top->right->bbox.get_min(), ray.get_direction()) < 0.0f)
						{
							stack[stack_size - 2] = top->right.get();
							stack[stack_size - 1] = top->left.get();
						}
					}
					else
					{
						for (int i = top->start; i < top->end; ++i)
						{
							if constexpr (isDereferenceable<Primitive>::value)
							{
								if (m_objects[i]->intersect(ray, intersection, min_distance))
								{
									min_distance = intersection.distance;
								}
							}
							else
							{
								if (m_objects[i].intersect(ray, intersection, min_distance))
								{
									min_distance = intersection.distance;
								}
							}
						}
					}
				}
			}

			return min_distance != max_distance;
		}

		template<typename Primitive>
		bool BVH<Primitive>::intersectShadowRay(const Ray& ray, float max_distance) const
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
							if constexpr (isDereferenceable<Primitive>::value)
							{
								if (m_objects[i]->intersectShadowRay(ray, max_distance))
								{
									return true;
								}
							}
							else
							{
								if (m_objects[i].intersectShadowRay(ray, max_distance))
								{
									return true;
								}
							}
						}

					}
				}
			}

			return false;
		}
	}
}