#ifndef __GLUE__GEOMETRY__TRANSFORMATION__
#define __GLUE__GEOMETRY__TRANSFORMATION__

#include "ray.h"

#include <glm\mat4x4.hpp>

namespace glue
{
	namespace geometry
	{
		class Transformation
		{
		public:
			Transformation();
			Transformation(const glm::mat4& transformation);

			void scale(const glm::vec3& scaling);
			void rotate(const glm::vec3& rotation_axis, float angle_in_degree);
			void translate(const glm::vec3& translation);
			Ray rayToObjectSpace(const Ray& ray) const;
			glm::vec3 pointToWorldSpace(const glm::vec3& point) const;
			glm::vec3 normalToWorldSpace(const glm::vec3& normal) const;

		private:
			glm::mat4 m_transformation;
			glm::mat4 m_inverse_transformation;
		};
	}
}

#endif