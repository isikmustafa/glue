#include "diffuse_arealight.h"
#include "..\core\real_sampler.h"
#include "..\geometry\object.h"

#include <glm\gtc\constants.hpp>

namespace glue
{
	namespace light
	{
		DiffuseArealight::DiffuseArealight(const std::shared_ptr<geometry::Object>& light_object, const glm::vec3& flux)
			: m_light_object(light_object)
			, m_pdf(1.0f / light_object->getSurfaceArea())
			, m_le(flux * glm::one_over_pi<float>() * m_pdf)
		{}

		geometry::Plane DiffuseArealight::samplePlane(core::UniformSampler& sampler) const
		{
			return m_light_object->samplePlane(sampler);
		}

		glm::vec3 DiffuseArealight::getLe() const
		{
			return m_le;
		}

		float DiffuseArealight::getPdf() const
		{
			return m_pdf;
		}

		bool DiffuseArealight::hasDeltaDistribution() const
		{
			return false;
		}
	}
}