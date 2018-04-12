#ifndef __GLUE__LIGHT__DIFFUSEAREALIGHT__
#define __GLUE__LIGHT__DIFFUSEAREALIGHT__

#include "..\geometry\ray.h"
#include "..\geometry\mesh.h"
#include "light.h"

#include <glm\vec3.hpp>
#include <memory>

namespace glue
{
	namespace light
	{
		class DiffuseArealight : public Light
		{
		public:
			DiffuseArealight(const std::shared_ptr<geometry::Mesh>& light_mesh, const glm::vec3& flux);

			geometry::Plane samplePlane(core::UniformSampler& sampler) const override;
			glm::vec3 getLe() const override;
			float getPdf() const override;
			bool hasDeltaDistribution() const override;

		private:
			std::shared_ptr<geometry::Mesh> m_light_mesh;
			float m_pdf;
			glm::vec3 m_le;
		};
	}
}

#endif