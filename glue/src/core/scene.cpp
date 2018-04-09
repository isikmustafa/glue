#include "scene.h"
#include "parser.h"
#include "..\geometry\triangle.h"
#include "..\geometry\transformation.h"
#include "..\geometry\bbox.h"
#include "timer.h"

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
				stream << element->GetText() << std::endl;
				stream >> pixel_filter;
			}
			else
			{
				pixel_filter = "BOX";
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
					auto datapath = child->FirstChildElement("Datapath");
					if (!datapath)
					{
						throw std::runtime_error("Error: Datapath is not found for the mesh!");
					}

					auto datapath_text = datapath->GetText();
					if (path_to_triangles.find(datapath_text) == path_to_triangles.end())
					{
						auto triangles = parser::parseTriangles(datapath);
						geometry::BVH bvh;

						glue::core::Timer timer; // delete include timer.h as well
						bvh.buildWithSAHSplit(triangles);
						std::cout << "BVH build time: " << timer.getTime() << std::endl;

						path_to_triangles.insert({ datapath_text, std::make_shared<std::vector<geometry::Triangle>>(std::move(triangles)) });
						path_to_bvh.insert({ datapath_text, std::make_shared<geometry::BVH>(std::move(bvh)) });
					}

					//Compute transformation.
					auto transformation_element = child->FirstChildElement("Transformation");
					auto transformation = parser::parseTransformation(transformation_element);

					//Compute bbox.
					geometry::BBox bbox;
					for (const auto& triangle : *path_to_triangles[datapath_text])
					{
						auto vertices = triangle.getVertices();
						bbox.extend(transformation.pointToWorldSpace(vertices[0]));
						bbox.extend(transformation.pointToWorldSpace(vertices[1]));
						bbox.extend(transformation.pointToWorldSpace(vertices[2]));
					}

					meshes.emplace_back(transformation, bbox, path_to_triangles[datapath_text], path_to_bvh[datapath_text]);

					child = child->NextSiblingElement("Mesh");
				}
			}

			bvh.buildWithMedianSplit(meshes);
		}
	}
}