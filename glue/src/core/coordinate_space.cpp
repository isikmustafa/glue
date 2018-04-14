#include "coordinate_space.h"

namespace glue
{
	namespace core
	{
		CoordinateSpace::CoordinateSpace(const glm::vec3& origin, const glm::vec3& z)
			: m_origin(origin)
		{
			auto w = glm::normalize(z);
			auto y = glm::abs(w.x) > 0.1f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
			auto u = glm::normalize(glm::cross(y, w));
			auto v = glm::cross(w, u); //Cross product of two normalized and orthogonal vectors is also a normalized one.

			m_transformation = glm::mat3(u, v, w);
			m_inverse_transformation = glm::transpose(m_transformation);
		}

		CoordinateSpace::CoordinateSpace(const glm::vec3& origin, const glm::vec3& z, const glm::vec3& y)
			: m_origin(origin)
		{
			auto w = glm::normalize(z);
			auto u = glm::normalize(glm::cross(y, w));
			auto v = glm::cross(w, u); //Cross product of two normalized and orthogonal vectors is also a normalized one.

			m_transformation = glm::mat3(u, v, w);
			m_inverse_transformation = glm::transpose(m_transformation);
		}

		glm::vec3 CoordinateSpace::vectorToLocalSpace(const glm::vec3& vector) const
		{
			return m_inverse_transformation * vector;
		}

		glm::vec3 CoordinateSpace::pointToLocalSpace(const glm::vec3& point) const
		{
			return m_inverse_transformation * (point - m_origin);
		}

		glm::vec3 CoordinateSpace::vectorToWorldSpace(const glm::vec3& vector) const
		{
			return m_transformation * vector;
		}

		glm::vec3 CoordinateSpace::pointToWorldSpace(const glm::vec3& point) const
		{
			return m_transformation * point + m_origin;
		}
	}
}