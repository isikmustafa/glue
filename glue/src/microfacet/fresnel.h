#ifndef __GLUE__MICROFACET__FRESNEL__
#define __GLUE__MICROFACET__FRESNEL__

#include <glm\vec3.hpp>

namespace glue
{
	namespace microfacet
	{
		namespace fresnel
		{
			struct Dielectric
			{
				float operator()(float n_ratio, float costheta);
			};

			struct Conductor
			{
				glm::vec3 operator()(const glm::vec3& n_ratio, const glm::vec3& k_ratio, float costheta);
			};
		}
	}
}

#endif