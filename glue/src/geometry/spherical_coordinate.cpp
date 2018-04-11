#include "spherical_coordinate.h"

#include <glm\geometric.hpp>
#include <glm\trigonometric.hpp>

namespace glue
{
	namespace geometry
	{
		OrthonormalBasis::OrthonormalBasis(const glm::vec3& p_w)
			: u(glm::normalize(glm::cross(glm::abs(p_w.x) > 0.1f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f), p_w)))
			, v(glm::cross(p_w, u))
			, w(p_w)
		{}

		OrthonormalBasis::OrthonormalBasis(const glm::vec3& p_u, const glm::vec3& p_v, const glm::vec3& p_w)
			: u(p_u)
			, v(p_v)
			, w(p_w)
		{}

		SphericalCoordinate::SphericalCoordinate(float p_radius, float p_theta, float p_phi)
			: radius(p_radius)
			, theta(p_theta)
			, phi(p_phi)
		{}

		glm::vec3 SphericalCoordinate::convertToCartesian(const OrthonormalBasis& basis) const
		{
			auto sin_theta = glm::sin(theta);
			auto magnitude = glm::vec3(glm::cos(phi) * sin_theta, glm::sin(phi) * sin_theta, glm::cos(theta)) * radius;

			return basis.u * magnitude.x + basis.v * magnitude.y + basis.w * magnitude.z;
		}
	}
}