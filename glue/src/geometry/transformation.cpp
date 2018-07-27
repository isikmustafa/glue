#include "transformation.h"
#include "..\xml\node.h"

#include <glm\gtx\transform.hpp>
#include <glm\gtx\euler_angles.hpp>
#include "..\core\tonemapper.h"

namespace glue
{
	namespace geometry
	{
		Transformation::Xml::Xml()
			: scaling(1.0f)
			, rotation(0.0f)
			, translation(0.0f)
		{}

		Transformation::Xml::Xml(const xml::Node& node)
		{
			node.parseChildText("Scaling", &scaling.x, 1.0f, &scaling.y, 1.0f, &scaling.z, 1.0f);
			node.parseChildText("Rotation", &rotation.x, 0.0f, &rotation.y, 0.0f, &rotation.z, 0.0f);
			node.parseChildText("Translation", &translation.x, 0.0f, &translation.y, 0.0f, &translation.z, 0.0f);

			if (node.parent().attribute("type") == std::string("Sphere"))
			{
				if (!(scaling.x == scaling.y && scaling.y == scaling.z))
				{
					node.child("Scaling").throwError("Non-uniform scaling is not allowed for spheres.");
				}
			}
		}
		
		Transformation::Transformation(const Transformation::Xml& xml)
			: m_transformation(glm::orientate4(glm::radians(glm::vec3(xml.rotation.x, xml.rotation.z, xml.rotation.y))) * glm::scale(xml.scaling))
			, m_inverse_transformation(glm::inverse(m_transformation))
			, m_translation(xml.translation)
		{}

		glm::vec3 Transformation::vectorToObjectSpace(const glm::vec3& vector) const
		{
			return m_inverse_transformation * vector;
		}

		glm::vec3 Transformation::vectorToWorldSpace(const glm::vec3& vector) const
		{
			return m_transformation * vector;
		}

		glm::vec3 Transformation::pointToObjectSpace(const glm::vec3& point) const
		{
			return m_inverse_transformation * (point - m_translation);
		}

		glm::vec3 Transformation::pointToWorldSpace(const glm::vec3& point) const
		{
			return m_transformation * point + m_translation;
		}

		glm::vec3 Transformation::normalToWorldSpace(const glm::vec3& normal) const
		{
			return glm::transpose(m_inverse_transformation) * normal;
		}

		Ray Transformation::rayToObjectSpace(const Ray& ray) const
		{
			return Ray(pointToObjectSpace(ray.get_origin()), vectorToObjectSpace(ray.get_direction()));
		}

		Plane Transformation::planeToWorldSpace(const Plane& plane) const
		{
			return Plane(pointToWorldSpace(plane.point), glm::normalize(normalToWorldSpace(plane.normal)));
		}
	}
}