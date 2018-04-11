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

		SphericalCoordinate::SphericalCoordinate(const glm::vec3& direction, const OrthonormalBasis& basis)
		{
			glm::vec3 uvw_coordinate(glm::dot(direction, basis.u), glm::dot(direction, basis.v), glm::dot(direction, basis.w));

			radius = glm::length(uvw_coordinate);
			theta = glm::atan(glm::sqrt(uvw_coordinate.x * uvw_coordinate.x + uvw_coordinate.y * uvw_coordinate.y) / uvw_coordinate.z);
			phi = glm::atan(uvw_coordinate.y / uvw_coordinate.x);
		}

		SphericalCoordinate::SphericalCoordinate(float p_radius, float p_theta, float p_phi)
			: radius(p_radius)
			, theta(p_theta)
			, phi(p_phi)
		{}

		glm::vec3 SphericalCoordinate::convertToCartesian(const OrthonormalBasis& basis) const
		{
			auto sin_theta = glm::sin(theta);
			auto uvw_coordinate = glm::vec3(glm::cos(phi) * sin_theta, glm::sin(phi) * sin_theta, glm::cos(theta)) * radius;

			return basis.u * uvw_coordinate.x + basis.v * uvw_coordinate.y + basis.w * uvw_coordinate.z;
		}
	}
}