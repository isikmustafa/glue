#include "intersection.h"
#include "ray.h"

#include <glm\mat2x2.hpp>

namespace glue
{
	namespace geometry
	{
		void Intersection::computeDifferentials(const geometry::Ray& ray)
		{
			if (ray.get_rx())
			{
				auto dpdx = plane.intersect(*ray.get_rx()) - plane.point;
				auto dpdy = plane.intersect(*ray.get_ry()) - plane.point;

				auto abs_normal = glm::abs(plane.normal);
				auto dim0 = 0;
				auto dim1 = 1;
				if (abs_normal.x > abs_normal.y && abs_normal.x > abs_normal.z)
				{
					dim0 = 1;
					dim1 = 2;
				}
				else if (abs_normal.y > abs_normal.z)
				{
					dim0 = 0;
					dim1 = 2;
				}

				auto lhs = glm::inverse(glm::mat2(dpdu[dim0], dpdu[dim1], dpdv[dim0], dpdv[dim1]));
				duvdx = lhs * glm::vec2(dpdx[dim0], dpdx[dim1]);
				duvdy = lhs * glm::vec2(dpdy[dim0], dpdy[dim1]);
			}
			else
			{
				duvdx = duvdy = glm::vec2(1e-6f);
			}
		}
	}
}