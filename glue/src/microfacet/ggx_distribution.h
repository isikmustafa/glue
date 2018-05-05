#ifndef __GLUE__MICROFACET__GGXDISTRIBUTION__
#define __GLUE__MICROFACET__GGXDISTRIBUTION__

#include "..\core\forward_decl.h"

#include <glm\vec3.hpp>

namespace glue
{
	namespace microfacet
	{
		class GGXDistribution
		{
		public:
			GGXDistribution(float roughness);

			glm::vec3 sampleWh(core::UniformSampler& sampler) const;
			float d(const glm::vec3& wh_tangent) const;
			float g1(const glm::vec3& wv_tangent, const glm::vec3& wh_tangent) const;

		private:
			float m_ag;
		};
	}
}

#endif