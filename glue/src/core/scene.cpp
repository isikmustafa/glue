#include "scene.h"
#include "pinhole_camera.h"
#include "tonemapper.h"
#include "uniform_sampler.h"
#include "..\geometry\triangle.h"
#include "..\geometry\transformation.h"
#include "..\geometry\bbox.h"
#include "..\geometry\bvh.h"
#include "..\geometry\mesh.h"
#include "..\light\diffuse_arealight.h"
#include "..\material\lambertian.h"
#include "..\material\oren_nayar.h"
#include "..\material\metal.h"
#include "..\material\dielectric.h"

#include <sstream>
#include <utility>
#include <glm\vec2.hpp>
#include <glm\vec3.hpp>
#include <glm\trigonometric.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace glue
{
	namespace core
	{
		void Scene::loadFromXML(const std::string& filepath)
		{
			tinyxml2::XMLDocument file;
			std::stringstream stream;

			if (file.LoadFile(filepath.c_str()))
			{
				throw std::runtime_error("Error: The xml file cannot be loaded!");
			}

			auto scene_element = file.FirstChildElement("Scene");
			if (!scene_element)
			{
				throw std::runtime_error("Error: Scene is not found!");
			}

			parseMeshes(scene_element);
			parseLights(scene_element);
			parseOutput(scene_element);
			parseCamera(scene_element);
			hdr_image = std::make_unique<HdrImage>(camera->get_screen_resolution().x, camera->get_screen_resolution().y);

			//BackgroundColor
			auto element = scene_element->FirstChildElement("BackgroundColor");
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
			element = scene_element->FirstChildElement("PixelFilter");
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
			element = scene_element->FirstChildElement("SampleCount");
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
			element = scene_element->FirstChildElement("SecondaryRayEpsilon");
			if (element)
			{
				stream << element->GetText() << std::endl;
				stream >> secondary_ray_epsilon;
			}
			else
			{
				secondary_ray_epsilon = 0.0001f;
			}

			debug_bvh.buildWithSAHSplit(debug_spheres);
			bvh.buildWithMedianSplit(meshes);
		}

		void Scene::parseCamera(tinyxml2::XMLElement* scene_element)
		{
			auto camera_element = scene_element->FirstChildElement("Camera");
			if (camera_element)
			{
				std::stringstream stream;
				auto child = camera_element->FirstChildElement("Position");
				stream << child->GetText() << std::endl;
				child = camera_element->FirstChildElement("Direction");
				stream << child->GetText() << std::endl;
				child = camera_element->FirstChildElement("Up");
				stream << child->GetText() << std::endl;
				child = camera_element->FirstChildElement("FovXY");
				stream << child->GetText() << std::endl;
				child = camera_element->FirstChildElement("Resolution");
				stream << child->GetText() << std::endl;
				child = camera_element->FirstChildElement("NearDistance");
				stream << child->GetText() << std::endl;

				glm::vec3 position;
				glm::vec3 direction;
				glm::vec3 up;
				glm::vec2 fov_xy;
				glm::ivec2 screen_resolution;
				float near_distance;

				stream >> position.x >> position.y >> position.z;
				stream >> direction.x >> direction.y >> direction.z;
				stream >> up.x >> up.y >> up.z;
				stream >> fov_xy.x >> fov_xy.y;
				stream >> screen_resolution.x >> screen_resolution.y;
				stream >> near_distance;

				auto s_right = glm::tan(glm::radians(fov_xy.x) * 0.5f) * near_distance;
				auto s_up = glm::tan(glm::radians(fov_xy.y) * 0.5f) * near_distance;
				camera = std::make_unique<PinholeCamera>(position, direction, up, glm::vec4(-s_right, s_right, -s_up, s_up), screen_resolution, near_distance);
			}
			else
			{
				throw std::runtime_error("Error: Camera is not found!");
			}
		}

		void Scene::parseOutput(tinyxml2::XMLElement* scene_element)
		{
			auto output_element = scene_element->FirstChildElement("Output");
			if (output_element)
			{
				std::stringstream stream;
				auto image_element = output_element->FirstChildElement("Image");
				while (image_element)
				{
					std::string image_name;

					stream << image_element->FirstChildElement("ImageName")->GetText();
					stream >> image_name;
					stream.clear();

					auto tonemapper_element = image_element->FirstChildElement("Tonemapper");
					auto tonemapper_type = tonemapper_element->Attribute("type");
					if (tonemapper_type == std::string("Clamp"))
					{
						float min;
						float max;

						stream << tonemapper_element->FirstChildElement("Min")->GetText();
						stream >> min;
						stream.clear();

						stream << tonemapper_element->FirstChildElement("Max")->GetText();
						stream >> max;
						stream.clear();

						output.emplace_back(std::make_pair(std::make_unique<Clamp>(min, max), image_name));
					}
					else if (tonemapper_type == std::string("GlobalReinhard"))
					{
						float key;
						float max_luminance;

						stream << tonemapper_element->FirstChildElement("Key")->GetText();
						stream >> key;
						stream.clear();
						stream << tonemapper_element->FirstChildElement("MaxLuminance")->GetText();
						stream >> max_luminance;
						stream.clear();

						output.emplace_back(std::make_pair(std::make_unique<GlobalReinhard>(key, max_luminance), image_name));
					}
					else
					{
						throw std::runtime_error("Error: Unknown Tonemapper type");
					}

					image_element = image_element->NextSiblingElement("Image");
				}
			}
			else
			{
				throw std::runtime_error("Error: Output is not found!");
			}
		}

		void Scene::parseMeshes(tinyxml2::XMLElement* scene_element)
		{
			UniformSampler sampler;
			auto element = scene_element->FirstChildElement("Objects");
			if (element)
			{
				auto child = element->FirstChildElement("Mesh");
				while (child)
				{
					parseMesh(child);
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
		}

		void Scene::parseLights(tinyxml2::XMLElement* scene_element)
		{
			std::stringstream stream;
			UniformSampler sampler;
			auto element = scene_element->FirstChildElement("Lights");
			if (element)
			{
				auto child = element->FirstChildElement("DiffuseArealight");
				while (child)
				{
					parseMesh(child);
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
		}

		void Scene::parseMesh(tinyxml2::XMLElement* mesh_element)
		{
			auto datapath = mesh_element->FirstChildElement("Datapath");
			if (!datapath)
			{
				throw std::runtime_error("Error: Datapath is not found for the mesh!");
			}

			auto datapath_text = datapath->GetText();
			if (m_path_to_triangles.find(datapath_text) == m_path_to_triangles.end())
			{
				auto triangles = parseTriangles(datapath);
				geometry::BVH bvh;

				bvh.buildWithSAHSplit(triangles);

				m_path_to_triangles.insert({ datapath_text, std::make_shared<std::vector<geometry::Triangle>>(std::move(triangles)) });
				m_path_to_bvh.insert({ datapath_text, std::make_shared<geometry::BVH>(std::move(bvh)) });
			}

			//Compute transformation.
			auto transformation = parseTransformation(mesh_element->FirstChildElement("Transformation"));

			//Compute bbox.
			geometry::BBox bbox;
			std::vector<float> triangle_areas;
			auto total_area = 0.0f;
			for (const auto& triangle : *m_path_to_triangles[datapath_text])
			{
				auto vertices = triangle.getVertices();
				auto v0 = transformation.pointToWorldSpace(vertices[0]);
				auto v1 = transformation.pointToWorldSpace(vertices[1]);
				auto v2 = transformation.pointToWorldSpace(vertices[2]);
				bbox.extend(v0);
				bbox.extend(v1);
				bbox.extend(v2);
				auto area = geometry::Triangle(v0, v1 - v0, v2 - v0).getSurfaceArea();
				triangle_areas.push_back(area);
				total_area += area;
			}

			meshes.push_back(std::make_shared<geometry::Mesh>(transformation, bbox, triangle_areas, total_area, m_path_to_triangles[datapath_text], m_path_to_bvh[datapath_text],
				parseBsdfMaterial(mesh_element->FirstChildElement("BsdfMaterial"))));
		}

		std::vector<geometry::Triangle> Scene::parseTriangles(tinyxml2::XMLElement* datapath_element)
		{
			std::vector<geometry::Triangle> triangles;

			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(datapath_element->GetText(),
				aiProcess_Triangulate |
				aiProcess_JoinIdenticalVertices);

			if (!scene)
			{
				throw std::runtime_error("Error: Assimp cannot load the model.");
			}

			if (scene->mNumMeshes > 1)
			{
				throw std::runtime_error("Unhandled case: More than one mesh to process in the same model.");
			}

			aiMesh* mesh = scene->mMeshes[0];
			int face_count = mesh->mNumFaces;
			glm::vec3 face_vertices[3];
			for (int i = 0; i < face_count; ++i)
			{
				const aiFace& face = mesh->mFaces[i];
				for (int k = 0; k < 3; ++k)
				{
					const aiVector3D& position = mesh->mVertices[face.mIndices[k]];
					face_vertices[k] = glm::vec3(position.x, position.y, position.z);
				}
				triangles.emplace_back(face_vertices[0], face_vertices[1] - face_vertices[0], face_vertices[2] - face_vertices[0]);
			}

			return triangles;
		}

		geometry::Transformation Scene::parseTransformation(tinyxml2::XMLElement* transformation_element)
		{
			std::stringstream stream;
			geometry::Transformation transformation;
			if (transformation_element)
			{
				auto scaling_element = transformation_element->FirstChildElement("Scaling");
				if (scaling_element)
				{
					glm::vec3 scaling;
					stream << scaling_element->GetText() << std::endl;
					stream >> scaling.x >> scaling.y >> scaling.z;
					transformation.scale(scaling);
				}
				auto rotation_element = transformation_element->FirstChildElement("Rotation");
				if (rotation_element)
				{
					glm::vec3 axis;
					float angle;
					stream << rotation_element->GetText() << std::endl;
					stream >> axis.x >> axis.y >> axis.z >> angle;
					transformation.rotate(glm::normalize(axis), angle);
				}
				auto translation_element = transformation_element->FirstChildElement("Translation");
				if (translation_element)
				{
					glm::vec3 translation;
					stream << translation_element->GetText() << std::endl;
					stream >> translation.x >> translation.y >> translation.z;
					transformation.translate(translation);
				}
			}

			return transformation;
		}

		std::unique_ptr<material::BsdfMaterial> Scene::parseBsdfMaterial(tinyxml2::XMLElement* bsdf_material_element)
		{
			if (bsdf_material_element)
			{
				std::stringstream stream;
				auto bsdf_type = bsdf_material_element->Attribute("type");

				if (bsdf_type == std::string("Lambertian"))
				{
					glm::vec3 kd;
					stream << bsdf_material_element->FirstChildElement("kd")->GetText();
					stream >> kd.x >> kd.y >> kd.z;

					return std::make_unique<material::Lambertian>(kd);
				}
				else if (bsdf_type == std::string("OrenNayar"))
				{
					glm::vec3 kd;
					float roughness;
					stream << bsdf_material_element->FirstChildElement("kd")->GetText();
					stream >> kd.x >> kd.y >> kd.z;
					stream.clear();
					stream << bsdf_material_element->FirstChildElement("Roughness")->GetText();
					stream >> roughness;

					return std::make_unique<material::OrenNayar>(kd, roughness);
				}
				else if (bsdf_type == std::string("Metal"))
				{
					glm::vec3 ior_n;
					glm::vec3 ior_k;
					float roughness;
					stream << bsdf_material_element->FirstChildElement("IorN")->GetText();
					stream >> ior_n.x >> ior_n.y >> ior_n.z;
					stream.clear();
					stream << bsdf_material_element->FirstChildElement("IorK")->GetText();
					stream >> ior_k.x >> ior_k.y >> ior_k.z;
					stream.clear();
					stream << bsdf_material_element->FirstChildElement("Roughness")->GetText();
					stream >> roughness;

					return std::make_unique<material::Metal>(ior_n, ior_k, roughness);
				}
				else if (bsdf_type == std::string("Dielectric"))
				{
					float ior_n;
					float roughness;
					stream << bsdf_material_element->FirstChildElement("IorN")->GetText();
					stream >> ior_n;
					stream.clear();
					stream << bsdf_material_element->FirstChildElement("Roughness")->GetText();
					stream >> roughness;

					return std::make_unique<material::Dielectric>(ior_n, roughness);
				}
				else
				{
					throw std::runtime_error("Error: Unknown BsdfMaterial type");
				}
			}
			else
			{
				return nullptr;
			}
		}
	}
}