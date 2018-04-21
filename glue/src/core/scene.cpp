#include "scene.h"
#include "parser.h"
#include "..\geometry\triangle.h"
#include "..\geometry\transformation.h"
#include "..\geometry\bbox.h"
#include "..\light\diffuse_arealight.h"
#include "..\core\uniform_sampler.h"

#include <sstream>
#include <utility>
#include <memory>
#include <unordered_map>
#include <tinyxml2.h>
#include <glm\vec2.hpp>
#include <glm\vec3.hpp>

namespace glue
{
	namespace core
	{
		void Scene::loadFromXML(const std::string& filepath)
		{
			tinyxml2::XMLDocument file;
			std::stringstream stream;

			auto res = file.LoadFile(filepath.c_str());
			if (res)
			{
				throw std::runtime_error("Error: The xml file cannot be loaded!");
			}

			auto root = file.FirstChild();
			if (!root)
			{
				throw std::runtime_error("Error: Root is not found!");
			}

			//BackgroundColor
			auto element = root->FirstChildElement("BackgroundColor");
			if (element)
			{
				stream << element->GetText() << std::endl;
				stream >> background_color.x >> background_color.y >> background_color.z;
			}
			else
			{
				background_color = glm::vec3(0.0f, 0.0f, 0.0f);
			}

			//Filter
			element = root->FirstChildElement("PixelFilter");
			if (element)
			{
				std::string filter_str;
				stream << element->GetText() << std::endl;
				stream >> filter_str;

				if (filter_str == "BOX")
				{
					pixel_filter = Filter::BOX;
				}
				else if (filter_str == "GAUSSIAN")
				{
					pixel_filter = Filter::GAUSSIAN;
				}
				else
				{
					throw std::runtime_error("Error: Unknown filter type for pixel!");
				}
			}
			else
			{
				pixel_filter = Filter::BOX;
			}

			//SampleCount
			element = root->FirstChildElement("SampleCount");
			if (element)
			{
				stream << element->GetText() << std::endl;
				stream >> sample_count;
			}
			else
			{
				sample_count = 1;
			}

			//SecondaryRayEpsilon
			element = root->FirstChildElement("SecondaryRayEpsilon");
			if (element)
			{
				stream << element->GetText() << std::endl;
				stream >> secondary_ray_epsilon;
			}
			else
			{
				secondary_ray_epsilon = 0.0001f;
			}

			//Get output
			output = parser::parseOutput(root->FirstChildElement("Output"));

			//Get camera
			camera = parser::parseCamera(root->FirstChildElement("Camera"));

			//Create the image
			hdr_image = std::make_unique<HdrImage>(camera->get_screen_resolution().x, camera->get_screen_resolution().y);

			//Get objects
			UniformSampler sampler;
			std::unordered_map<std::string, std::shared_ptr<std::vector<geometry::Triangle>>> path_to_triangles;
			std::unordered_map<std::string, std::shared_ptr<geometry::BVH>> path_to_bvh;
			element = root->FirstChildElement("Objects");
			if (element)
			{
				auto child = element->FirstChildElement("Mesh");
				while (child)
				{
					meshes.push_back(std::make_shared<geometry::Mesh>(parser::parseMesh(child, path_to_triangles, path_to_bvh)));
					const auto& mesh = meshes[meshes.size() - 1];

					auto att = child->Attribute("displayRandomSamples");
					if (att)
					{
						auto num_of_samples = std::atoi(att);
						for (int i = 0; i < num_of_samples; ++i)
						{
							debug_spheres.emplace_back(mesh->samplePlane(sampler).point, glm::sqrt(mesh->getSurfaceArea() / num_of_samples) / 5.0f);
						}
					}

					child = child->NextSiblingElement("Mesh");
				}
			}

			//Get Lights
			element = root->FirstChildElement("Lights");
			if (element)
			{
				auto child = element->FirstChildElement("DiffuseArealight");
				while (child)
				{
					meshes.push_back(std::make_shared<geometry::Mesh>(parser::parseMesh(child, path_to_triangles, path_to_bvh)));
					const auto& mesh = meshes[meshes.size() - 1];

					auto att = child->Attribute("displayRandomSamples");
					if (att)
					{
						auto num_of_samples = std::atoi(att);
						for (int i = 0; i < num_of_samples; ++i)
						{
							debug_spheres.emplace_back(mesh->samplePlane(sampler).point, glm::sqrt(mesh->getSurfaceArea() / num_of_samples) / 5.0f);
						}
					}

					glm::vec3 flux;
					stream << child->FirstChildElement("Flux")->GetText() << std::endl;
					stream >> flux.x >> flux.y >> flux.z;

					lights.push_back(std::make_shared<light::DiffuseArealight>(mesh, flux));
					const auto& light = lights[lights.size() - 1];

					light_meshes[mesh.get()] = light.get();

					child = child->NextSiblingElement("DiffuseArealight");
				}
			}

			debug_bvh.buildWithSAHSplit(debug_spheres);
			bvh.buildWithMedianSplit(meshes);
		}
	}
}