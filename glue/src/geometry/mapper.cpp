#include "mapper.h"
#include "..\geometry\spherical_coordinate.h"
#include "intersection.h"

#include <glm\trigonometric.hpp>
#include <glm\gtc\constants.hpp>
#include <glm\mat2x2.hpp>
#include <glm\mat3x2.hpp>

namespace glue
{
	namespace geometry
	{
		UVMapper::UVMapper(const glm::vec3& edge1, const glm::vec3& edge2, const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2)
			: m_uv0(uv0)
			, m_uv1(uv1)
			, m_uv2(uv2)
		{
			//glm::transpose is used only for brevity. If this constructor will be called on hot areas, do not use it and make it more efficient.
			auto dp = glm::transpose(glm::inverse(glm::transpose(glm::mat2(m_uv0 - m_uv2, m_uv1 - m_uv2))) * glm::transpose(glm::mat2x3(-edge2, edge1 - edge2)));
			m_dpdu = dp[0];
			m_dpdv = dp[1];
		}

		Mapper::Values UVMapper::map(const glm::vec3& cartesian, const glm::vec3& barycentric) const
		{
			Values values;

			values.uv = m_uv0 * barycentric.x + m_uv1 * barycentric.y + m_uv2 * barycentric.z;
			values.dpdu = m_dpdu;
			values.dpdv = m_dpdv;

			return values;
		}

		Mapper::Values UVMapper::mapOnlyUV(const glm::vec3 & cartesian, const glm::vec3 & barycentric) const
		{
			Values values;

			values.uv = m_uv0 * barycentric.x + m_uv1 * barycentric.y + m_uv2 * barycentric.z;

			return values;
		}

		SphericalMapper::SphericalMapper()
		{}

		Mapper::Values SphericalMapper::map(const glm::vec3& cartesian, const glm::vec3& barycentric) const
		{
			Values values;

			geometry::SphericalCoordinate spherical_position(cartesian);
			values.uv = glm::vec2(spherical_position.phi * glm::one_over_two_pi<float>(), spherical_position.theta * glm::one_over_pi<float>());
			values.dpdu = glm::vec3(-glm::two_pi<float>() * cartesian.y, glm::two_pi<float>() * cartesian.x, 0.0f);
			values.dpdv = glm::vec3(cartesian.z * glm::cos(spherical_position.phi), cartesian.z * glm::sin(spherical_position.phi), -spherical_position.radius * glm::sin(spherical_position.theta)) * glm::pi<float>();

			return values;
		}

		Mapper::Values SphericalMapper::mapOnlyUV(const glm::vec3 & cartesian, const glm::vec3 & barycentric) const
		{
			Values values;

			geometry::SphericalCoordinate spherical_position(cartesian);
			values.uv = glm::vec2(spherical_position.phi * glm::one_over_two_pi<float>(), spherical_position.theta * glm::one_over_pi<float>());

			return values;
		}
	}
}