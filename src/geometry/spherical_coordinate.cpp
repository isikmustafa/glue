#include "spherical_coordinate.h"

#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>

namespace glue
{
	namespace geometry
	{
		SphericalCoordinate::SphericalCoordinate(const glm::vec3& cartesian_coordinate)
			: radius(glm::length(cartesian_coordinate))
			, theta(glm::atan(glm::sqrt(cartesian_coordinate.x * cartesian_coordinate.x + cartesian_coordinate.y * cartesian_coordinate.y), cartesian_coordinate.z))
			, phi(glm::atan(cartesian_coordinate.y, cartesian_coordinate.x) + (cartesian_coordinate.y < 0.0f ? glm::two_pi<float>() : 0.0f))
		{}

		SphericalCoordinate::SphericalCoordinate(float p_radius, float p_theta, float p_phi)
			: radius(p_radius)
			, theta(p_theta)
			, phi(p_phi)
		{}

		glm::vec3 SphericalCoordinate::toCartesianCoordinate() const
		{
			auto sin_theta = glm::sin(theta);
			return glm::vec3(glm::cos(phi) * sin_theta, glm::sin(phi) * sin_theta, glm::cos(theta)) * radius;
		}
	}
}