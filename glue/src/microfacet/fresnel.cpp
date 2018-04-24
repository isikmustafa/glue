#include "fresnel.h"

#include <glm\geometric.hpp>

namespace glue
{
	namespace microfacet
	{
		namespace fresnel
		{
			//This fresnel implementation for dielectric does not consider wavelength varying index of refractions.
			float Dielectric::operator()(float no_over_ni, float costheta)
			{
				auto g = no_over_ni * no_over_ni - 1.0f + costheta * costheta;

				//In the case of total internal reflection, return 1.
				if (g < 0.0f)
				{
					return 1.0f;
				}

				g = glm::sqrt(g);
				auto g_minus_c = g - costheta;
				auto g_plus_c = g + costheta;
				auto temp_div = (costheta * g_plus_c - 1.0f) / (costheta * g_minus_c + 1.0f);

				return 0.5f * (g_minus_c * g_minus_c) * (1.0f + temp_div * temp_div) / (g_plus_c * g_plus_c);
			}

			glm::vec3 Conductor::operator()(const glm::vec3& no_over_ni, const glm::vec3& ko_over_ki, float costheta)
			{
				/*From https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations*/
				auto cos2theta = costheta * costheta;
				auto sin2theta = 1.0f - cos2theta;
				auto n2 = no_over_ni * no_over_ni;
				auto k2 = ko_over_ki * ko_over_ki;

				auto t0 = n2 - k2 - sin2theta;
				auto a2_plus_b2 = glm::sqrt(t0 * t0 + 4.0f * n2 * k2);
				auto t1 = a2_plus_b2 + cos2theta;
				auto a = glm::sqrt(0.5f * (a2_plus_b2 + t0));
				auto t2 = 2.0f * a * costheta;
				auto rs = (t1 - t2) / (t1 + t2);

				auto t3 = cos2theta * a2_plus_b2 + sin2theta * sin2theta;
				auto t4 = t2 * sin2theta;
				auto rp = rs * (t3 - t4) / (t3 + t4);

				return 0.5f * (rp + rs);
			}
		}
	}
}