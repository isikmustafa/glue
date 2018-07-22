#ifndef __GLUE__GEOMETRY__MAPPER__
#define __GLUE__GEOMETRY__MAPPER__

#include <glm\vec2.hpp>
#include <glm\vec3.hpp>

namespace glue
{
	namespace geometry
	{
		class Mapper
		{
		public:
			struct Values
			{
				glm::vec2 uv;
				glm::vec3 dpdu;
				glm::vec3 dpdv;
			};

		public:
			virtual ~Mapper() {}

			virtual Values map(const glm::vec3& cartesian, const glm::vec3& barycentric) const = 0;
		};

		class UVMapper : public Mapper
		{
		public:
			UVMapper(const glm::vec3& edge1, const glm::vec3& edge2, const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2);

			Values map(const glm::vec3& cartesian, const glm::vec3& barycentric) const override;

		private:
			glm::vec3 m_dpdu;
			glm::vec3 m_dpdv;
			glm::vec2 m_uv0;
			glm::vec2 m_uv1;
			glm::vec2 m_uv2;
		};

		class SphericalMapper : public Mapper
		{
		public:
			SphericalMapper();

			Values map(const glm::vec3& cartesian, const glm::vec3& barycentric) const override;
		};
	}
}

#endif