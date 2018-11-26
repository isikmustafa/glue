#include "scene.h"
#include "real_sampler.h"
#include "timer.h"
#include "../geometry/sphere.h"
#include "../material/lambertian.h"
#include "../texture/constant_texture.h"
#include "../xml/node.h"

#include <iostream>

namespace glue
{
	namespace core
	{
		Scene::Xml::Xml(const xml::Node& node)
		{
			node.parseChildText("BackgroundRadiance", &background_radiance.x, 0.0f, &background_radiance.y, 0.0f, &background_radiance.z, 0.0f);
			node.parseChildText("SecondaryRayEpsilon", &secondary_ray_epsilon, 1e-4f);
			integrator = integrator::Integrator::Xml::factory(node.child("Integrator", true));
			for (auto output = node.child("Output"); output; output = output.next())
			{
				outputs.push_back(Output::Xml::factory(output));
			}
			camera = std::make_unique<PinholeCamera::Xml>(node.child("Camera", true));
			for (auto object = node.child("Object"); object; object = object.next())
			{
				objects.push_back(geometry::Object::Xml::factory(object));
			}
			for (auto light = node.child("Light"); light; light = light.next())
			{
				lights.push_back(light::Light::Xml::factory(light));
			}
		}

		Scene::Scene(const Scene::Xml& xml)
			: environment_light(nullptr)
		{
			background_radiance = xml.background_radiance;
			secondary_ray_epsilon = xml.secondary_ray_epsilon;
			m_integrator = xml.integrator->create();
			for (const auto& output_xml : xml.outputs)
			{
				m_outputs.push_back(output_xml->create());
			}
			camera = std::make_unique<PinholeCamera>(*xml.camera);
			m_image = std::make_unique<Image>(camera->get_resolution().x, camera->get_resolution().y);
			for (const auto& object_xml : xml.objects)
			{
				auto object = object_xml->create();

				//Add debug spheres
				if (object_xml->attributes.find("displayRandomSamples") != object_xml->attributes.end())
				{
					UniformSampler sampler;
					auto number_of_samples = std::stoi(object_xml->attributes["displayRandomSamples"]);
					for (int i = 0; i < number_of_samples; ++i)
					{
						auto radius = glm::sqrt(object->getSurfaceArea() / number_of_samples) * 0.2f;
						auto center = object->samplePlane(sampler).point;
						geometry::Transformation::Xml transformation;
						auto bsdf_material = std::make_unique<material::Lambertian::Xml>(std::make_unique<texture::ConstantTexture::Xml>(glm::vec3(0.8f, 0.0f, 0.0f)));
						geometry::Sphere::Xml debug_sphere_xml(radius, center, transformation, std::move(bsdf_material));

						bvh.addObject(debug_sphere_xml.create());
					}
				}

				bvh.addObject(std::move(object));
			}
			for (const auto& light_xml : xml.lights)
			{
				auto light = light_xml->create();
				auto object = light->getObject();
				if (object)
				{
					object_to_light[object.get()] = light.get();
					bvh.addObject(object);
				}
				lights.push_back(std::move(light));

				if (light_xml->attributes["type"] == "EnvironmentLight")
				{
					if (environment_light)
					{
						throw std::runtime_error("Error: There can be at most one EnvironmentLight");
					}
					environment_light = lights.back();
				}
			}

			if (bvh.get_objects().size() < 1024)
			{
				bvh.buildWithMedianSplit();
			}
			else
			{
				bvh.buildWithSAHSplit();
			}
		}

		void Scene::render()
		{
			Timer timer;
			timer.start();
			m_integrator->integrate(*this, *m_image);
			std::cout << "Render time: " << timer.getTime() << std::endl;

			int size = m_outputs.size();
			int i;
			#pragma omp parallel for
			for (i = 0; i < size; ++i)
			{
				m_outputs[i]->save(*m_image);
			}
		}

		glm::vec3 Scene::getBackgroundRadiance(const glm::vec3& direction, bool light_explicitly_sampled) const
		{
			if (environment_light)
			{
				if (!light_explicitly_sampled)
				{
					return environment_light->getLe(direction, glm::vec3(0.0f), 0.0f);
				}
				else
				{
					return glm::vec3(0.0f);
				}
			}
			else
			{
				return background_radiance;
			}
		}
	}
}