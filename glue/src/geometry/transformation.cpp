#include "transformation.h"

#include <glm/gtx/transform.hpp>

namespace glue
{
	namespace geometry
	{
		Transformation::Transformation()
			: m_transformation(1.0f)
			, m_inverse_transformation(1.0f)
		{}

		Transformation::Transformation(const glm::mat4& transformation)
			: m_transformation(transformation)
			, m_inverse_transformation(glm::inverse(transformation))
		{}

		void Transformation::scale(const glm::vec3& scaling)
		{
			m_transformation = glm::scale(scaling) * m_transformation;
			m_inverse_transformation = glm::inverse(m_transformation);
		}

		void Transformation::rotate(const glm::vec3& rotation_axis, float angle_in_degrees)
		{
			m_transformation = glm::rotate(glm::radians(angle_in_degrees), rotation_axis) * m_transformation;
			m_inverse_transformation = glm::inverse(m_transformation);
		}

		void Transformation::translate(const glm::vec3& translation)
		{
			m_transformation = glm::translate(translation) * m_transformation;
			m_inverse_transformation = glm::inverse(m_transformation);
		}

		Ray Transformation::rayToObjectSpace(const Ray& ray) const
		{
			return Ray(m_inverse_transformation * glm::vec4(ray.get_origin(), 1.0f), m_inverse_transformation * glm::vec4(ray.get_direction(), 0.0f));
		}

		glm::vec3 Transformation::pointToWorldSpace(const glm::vec3& point) const
		{
			return m_transformation * glm::vec4(point, 1.0f);
		}

		glm::vec3 Transformation::normalToWorldSpace(const glm::vec3& normal) const
		{
			return glm::transpose(m_inverse_transformation) * glm::vec4(normal, 0.0f);
		}
	}
}