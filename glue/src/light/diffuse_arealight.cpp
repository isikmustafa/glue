#include "diffuse_arealight.h"
#include "..\core\real_sampler.h"
#include "..\geometry\object.h"
#include "..\xml\node.h"

#include <glm\gtc\constants.hpp>

namespace glue
{
	namespace light
	{
		DiffuseArealight::Xml::Xml(const xml::Node& node)
		{
			node.parseChildText("Flux", &flux.x, &flux.y, &flux.z);
			object = geometry::Object::Xml::factory(node.child("Object", true));
		}

		std::unique_ptr<Light> DiffuseArealight::Xml::create() const
		{
			return std::make_unique<DiffuseArealight>(*this);
		}

		DiffuseArealight::DiffuseArealight(const DiffuseArealight::Xml& xml)
			: m_object(xml.object->create())
			, m_pdf(1.0f / m_object->getSurfaceArea())
			, m_le(xml.flux * glm::one_over_pi<float>() * m_pdf)
		{}

		geometry::Plane DiffuseArealight::samplePlane(core::UniformSampler& sampler) const
		{
			return m_object->samplePlane(sampler);
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

		std::shared_ptr<geometry::Object> DiffuseArealight::getObject() const
		{
			return m_object;
		}
	}
}