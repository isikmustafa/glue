#include "sphere.h"
#include "ray.h"
#include "intersection.h"
#include "spherical_mapper.h"
#include "..\core\real_sampler.h"
#include "..\geometry\spherical_coordinate.h"
#include "..\xml\node.h"

#include <glm\geometric.hpp>
#include <glm\trigonometric.hpp>
#include <glm\gtc\constants.hpp>

namespace glue
{
	namespace geometry
	{
		Sphere::Xml::Xml(const xml::Node& node)
		{
			attributes = node.attributes();
			node.parseChildText("Radius", &radius);
			node.parseChildText("Center", &center.x, &center.y, &center.z);
			transformation = node.child("Transformation") ? Transformation::Xml(node.child("Transformation")) : Transformation::Xml();
			bsdf_material = node.parent().value() == std::string("Light") ? nullptr : material::BsdfMaterial::Xml::factory(node.child("BsdfMaterial", true));
		}

		Sphere::Xml::Xml(float p_radius, glm::vec3 p_center, const Transformation::Xml& p_transformation, std::unique_ptr<material::BsdfMaterial::Xml> p_bsdf_material)
			: radius(p_radius)
			, center(p_center)
			, transformation(p_transformation)
			, bsdf_material(std::move(p_bsdf_material))
		{}

		std::unique_ptr<Object> Sphere::Xml::create() const
		{
			return std::make_unique<Sphere>(*this);
		}

		Sphere::Sphere(const Sphere::Xml& xml)
			: m_transformation(xml.transformation)
			, m_bsdf_material(xml.bsdf_material ? xml.bsdf_material->create() : nullptr)
		{
			auto t_radius = xml.radius * xml.transformation.scaling.x;
			auto t_center = xml.center + xml.transformation.translation;

			auto transformation_xml = xml.transformation;
			transformation_xml.scaling = glm::vec3(t_radius);
			transformation_xml.translation = t_center;

			m_transformation = Transformation(transformation_xml);
			m_bbox = geometry::BBox(t_center - transformation_xml.scaling, t_center + transformation_xml.scaling);
			m_area = 4.0f * glm::pi<float>() * t_radius * t_radius;
		}

		geometry::Plane Sphere::samplePlane(core::UniformSampler& sampler) const
		{
			auto point = SphericalCoordinate(1.0f, glm::acos(1.0f - 2.0f * sampler.sample()), glm::two_pi<float>() * sampler.sample()).toCartesianCoordinate();

			return m_transformation.planeToWorldSpace(Plane(point, point));
		}

		float Sphere::getSurfaceArea() const
		{
			return m_area;
		}

		BBox Sphere::getBBox() const
		{
			return m_bbox;
		}

		glm::vec2 Sphere::getBoundsOnAxis(int axis) const
		{
			return glm::vec2(m_bbox.get_min()[axis], m_bbox.get_max()[axis]);
		}

		bool Sphere::intersect(const Ray& ray, Intersection& intersection, float max_distance) const
		{
			//Analytic solution.
			auto transformed_ray = m_transformation.rayToObjectSpace(ray);
			auto a = glm::dot(transformed_ray.get_direction(), transformed_ray.get_direction());
			auto b = 2 * glm::dot(transformed_ray.get_direction(), transformed_ray.get_origin());
			auto c = glm::dot(transformed_ray.get_origin(), transformed_ray.get_origin()) - 1.0f;

			auto distance = 0.0f;
			auto discriminant = b * b - 4 * a * c;
			if (discriminant >= 0.0f)
			{
				auto delta = glm::sqrt(discriminant);
				auto div = 0.5f / a; //To prevent cancelling out.
				distance = -div * b - div * delta;

				if (distance < 0.0f)
				{
					distance = -div * b + div * delta;
				}
			}

			if (distance > 0.0f && distance < max_distance)
			{
				intersection.plane.point = ray.getPoint(distance);
				auto local_point = transformed_ray.getPoint(distance);
				intersection.plane.normal = glm::normalize(m_transformation.normalToWorldSpace(local_point));

				SphericalMapper mapper(local_point);
				intersection.uv = mapper.uv;
				intersection.dpdu = m_transformation.vectorToWorldSpace(mapper.dpdu);
				intersection.dpdv = m_transformation.vectorToWorldSpace(mapper.dpdv);

				intersection.distance = distance;
				intersection.object = this;
				intersection.bsdf_material = m_bsdf_material.get();

				return true;
			}

			return false;
		}

		bool Sphere::intersectShadowRay(const Ray& ray, float max_distance) const
		{
			//Analytic solution.
			auto transformed_ray = m_transformation.rayToObjectSpace(ray);
			auto a = glm::dot(transformed_ray.get_direction(), transformed_ray.get_direction());
			auto b = 2 * glm::dot(transformed_ray.get_direction(), transformed_ray.get_origin());
			auto c = glm::dot(transformed_ray.get_origin(), transformed_ray.get_origin()) - 1.0f;

			auto distance = 0.0f;
			auto discriminant = b * b - 4 * a * c;
			if (discriminant >= 0.0f)
			{
				auto delta = glm::sqrt(discriminant);
				auto div = 0.5f / a; //To prevent cancelling out.
				distance = -div * b - div * delta;

				if (distance < 0.0f)
				{
					distance = -div * b + div * delta;
				}
			}

			return (distance > 0.0f && distance < max_distance);
		}
	}
}