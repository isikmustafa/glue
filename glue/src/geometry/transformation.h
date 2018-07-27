#ifndef __GLUE__GEOMETRY__TRANSFORMATION__
#define __GLUE__GEOMETRY__TRANSFORMATION__

#include "ray.h"
#include "plane.h"
#include "..\core\forward_decl.h"

#include <glm\mat4x4.hpp>

namespace glue
{
	namespace geometry
	{
		class Transformation
		{
		public:
			//Xml structure of the class.
			struct Xml
			{
				glm::vec3 scaling;
				glm::vec3 rotation;
				glm::vec3 translation;

				Xml();
				explicit Xml(const xml::Node& node);
			};

		public:
			Transformation(const Transformation::Xml& xml);

			glm::vec3 vectorToObjectSpace(const glm::vec3& vector) const;
			glm::vec3 vectorToWorldSpace(const glm::vec3& vector) const;
			glm::vec3 pointToObjectSpace(const glm::vec3& point) const;
			glm::vec3 pointToWorldSpace(const glm::vec3& point) const;
			glm::vec3 normalToWorldSpace(const glm::vec3& normal) const;
			Ray rayToObjectSpace(const Ray& ray) const;
			Plane planeToWorldSpace(const Plane& plane) const;

		private:
			glm::mat3 m_transformation;
			glm::mat3 m_inverse_transformation;
			glm::vec3 m_translation;
		};
	}
}

#endif