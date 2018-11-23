#ifndef __GLUE__MICROFACET__FRESNEL__
#define __GLUE__MICROFACET__FRESNEL__

#include <glm/vec3.hpp>

namespace glue
{
	namespace microfacet
	{
		namespace fresnel
		{
			struct Dielectric
			{
				float operator()(float no_over_ni, float costheta);
			};

			struct Conductor
			{
				glm::vec3 operator()(const glm::vec3& no_over_ni, const glm::vec3& ko_over_ki, float costheta);
			};
		}
	}
}

#endif