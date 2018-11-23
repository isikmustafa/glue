#ifndef __GLUE__MICROFACET__BECKMANNDISTRIBUTION__
#define __GLUE__MICROFACET__BECKMANNDISTRIBUTION__

#include "../core/forward_decl.h"

#include <glm/vec3.hpp>

namespace glue
{
	namespace microfacet
	{
		class BeckmannDistribution
		{
		public:
			explicit BeckmannDistribution(float roughness);

			//Samples from distribution of normals.
			glm::vec3 sampleWhWmlt07(core::UniformSampler& sampler) const;
			float pdfWmlt07(const glm::vec3& wh_tangent) const;
			//Samples from distribution of visible normals and causes much less variance at grazing angles.
			glm::vec3 sampleWhHd14(const glm::vec3& wi_tangent, core::UniformSampler& sampler) const;
			float pdfHd14(const glm::vec3& wi_tangent, const glm::vec3& wh_tangent) const;
			//Distribution and Masking-Shadowing functions.
			float d(const glm::vec3& wh_tangent) const;
			float g1(const glm::vec3& wv_tangent, const glm::vec3& wh_tangent) const;

		private:
			float m_ab;
		};
	}
}

#endif