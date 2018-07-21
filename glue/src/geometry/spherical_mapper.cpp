#include "spherical_mapper.h"
#include "..\geometry\spherical_coordinate.h"
#include "intersection.h"

#include <glm\trigonometric.hpp>
#include <glm\gtc\constants.hpp>

namespace glue
{
	namespace geometry
	{
		SphericalMapper::SphericalMapper(const glm::vec3& point)
		{
			geometry::SphericalCoordinate spherical_position(point);

			uv = glm::vec2(spherical_position.phi * glm::one_over_two_pi<float>(), spherical_position.theta * glm::one_over_pi<float>());
			dpdu = glm::vec3(-glm::two_pi<float>() * point.y, glm::two_pi<float>() * point.x, 0.0f);
			dpdv = glm::vec3(point.z * glm::cos(spherical_position.phi), point.z * glm::sin(spherical_position.phi), -spherical_position.radius * glm::sin(spherical_position.theta)) * glm::pi<float>();
		}
	}
}