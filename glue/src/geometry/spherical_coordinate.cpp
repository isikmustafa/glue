#include "spherical_coordinate.h"

#include <glm\geometric.hpp>
#include <glm\trigonometric.hpp>

namespace glue
{
	namespace geometry
	{
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