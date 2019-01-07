#ifndef __GLUE__CORE__COORDINATESPACE__
#define __GLUE__CORE__COORDINATESPACE__

#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>

namespace glue
{
	namespace core
	{
		class CoordinateSpace
		{
		public:
			CoordinateSpace() = default;
			CoordinateSpace(const glm::vec3& origin, const glm::vec3& z);
			CoordinateSpace(const glm::vec3& origin, const glm::vec3& z, const glm::vec3& y);

			glm::vec3 vectorToLocalSpace(const glm::vec3& vector) const;
			glm::vec3 pointToLocalSpace(const glm::vec3& point) const;
			glm::vec3 vectorToWorldSpace(const glm::vec3& vector) const;
			glm::vec3 pointToWorldSpace(const glm::vec3& point) const;

			const glm::vec3& get_origin() const { return m_origin; }

		private:
			glm::mat3 m_transformation;
			glm::mat3 m_inverse_transformation;
			glm::vec3 m_origin;
		};
	}
}

#endif