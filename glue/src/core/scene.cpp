#include "scene.h"
#include "parser.h"
#include "..\geometry\triangle.h"
#include "..\geometry\transformation.h"
#include "..\geometry\bbox.h"
#include "..\light\diffuse_arealight.h"

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

			//ImageName
			auto element = root->FirstChildElement("ImageName");
			if (element)
			{
				stream << element->GetText() << std::endl;
				stream >> image_name;
			}
			else
			{
				image_name = "unknown.png";
			}

			//BackgroundColor
			element = root->FirstChildElement("BackgroundColor");
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

			//Get camera
			element = root->FirstChildElement("Camera");
			camera = parser::parseCamera(element);

			//Get objects
			std::unordered_map<std::string, std::shared_ptr<std::vector<geometry::Triangle>>> path_to_triangles;
			std::unordered_map<std::string, std::shared_ptr<geometry::BVH>> path_to_bvh;
			element = root->FirstChildElement("Objects");
			if (element)
			{
				auto child = element->FirstChildElement("Mesh");
				while (child)
				{
					meshes.push_back(std::make_shared<geometry::Mesh>(parser::parseMesh(child, path_to_triangles, path_to_bvh)));
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

					glm::vec3 flux;
					stream << child->FirstChildElement("Flux")->GetText() << std::endl;
					stream >> flux.x >> flux.y >> flux.z;

					lights.push_back(std::make_shared<light::DiffuseArealight>(meshes[meshes.size() - 1], flux));
					light_meshes[meshes[meshes.size() - 1].get()] = lights[lights.size() - 1].get();

					child = child->NextSiblingElement("DiffuseArealight");
				}
			}

			bvh.buildWithMedianSplit(meshes);
		}
	}
}