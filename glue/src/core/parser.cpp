#include "parser.h"
#include "..\geometry\bvh.h"
#include "..\material\lambertian.h"
#include "..\material\oren_nayar.h"
#include "..\material\metal.h"

#include <sstream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm\trigonometric.hpp>

namespace glue
{
	namespace core
	{
		namespace parser
		{
			std::unique_ptr<PinholeCamera> parseCamera(tinyxml2::XMLElement* camera_element)
			{
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
					return std::make_unique<PinholeCamera>(position, direction, up, glm::vec4(-s_right, s_right, -s_up, s_up), screen_resolution, near_distance);
				}
				else
				{
					throw std::runtime_error("Error: Camera is not found!");
				}
			}

			std::vector<geometry::Triangle> parseTriangles(tinyxml2::XMLElement* datapath_element)
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

			geometry::Transformation parseTransformation(tinyxml2::XMLElement* transformation_element)
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

			geometry::Mesh parseMesh(tinyxml2::XMLElement* mesh_element,
				std::unordered_map<std::string, std::shared_ptr<std::vector<geometry::Triangle>>>& path_to_triangles,
				std::unordered_map<std::string, std::shared_ptr<geometry::BVH>>& path_to_bvh)
			{
				auto datapath = mesh_element->FirstChildElement("Datapath");
				if (!datapath)
				{
					throw std::runtime_error("Error: Datapath is not found for the mesh!");
				}

				auto datapath_text = datapath->GetText();
				if (path_to_triangles.find(datapath_text) == path_to_triangles.end())
				{
					auto triangles = parser::parseTriangles(datapath);
					geometry::BVH bvh;

					bvh.buildWithSAHSplit(triangles);

					path_to_triangles.insert({ datapath_text, std::make_shared<std::vector<geometry::Triangle>>(std::move(triangles)) });
					path_to_bvh.insert({ datapath_text, std::make_shared<geometry::BVH>(std::move(bvh)) });
				}

				//Compute transformation.
				auto transformation = parser::parseTransformation(mesh_element->FirstChildElement("Transformation"));

				//Compute bbox.
				geometry::BBox bbox;
				std::vector<float> triangle_areas;
				auto total_area = 0.0f;
				for (const auto& triangle : *path_to_triangles[datapath_text])
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

				return geometry::Mesh(transformation, bbox, triangle_areas, total_area, path_to_triangles[datapath_text], path_to_bvh[datapath_text],
					parseBsdfMaterial(mesh_element->FirstChildElement("BsdfMaterial")));
			}

			std::unique_ptr<material::BsdfMaterial> parseBsdfMaterial(tinyxml2::XMLElement* bsdf_material_element)
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
						glm::vec3 n;
						glm::vec3 k;
						float roughness;
						stream << bsdf_material_element->FirstChildElement("n")->GetText();
						stream >> n.x >> n.y >> n.z;
						stream.clear();
						stream << bsdf_material_element->FirstChildElement("k")->GetText();
						stream >> k.x >> k.y >> k.z;
						stream.clear();
						stream << bsdf_material_element->FirstChildElement("Roughness")->GetText();
						stream >> roughness;

						return std::make_unique<material::Metal>(n, k, roughness);
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
			std::vector<std::pair<std::unique_ptr<Tonemapper>, std::string>> parseOutput(tinyxml2::XMLElement* output_element)
			{
				std::vector<std::pair<std::unique_ptr<Tonemapper>, std::string>> output;

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

							stream << tonemapper_element->FirstChildElement("Key")->GetText();
							stream >> key;
							stream.clear();

							output.emplace_back(std::make_pair(std::make_unique<GlobalReinhard>(key), image_name));
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

				return output;
			}
		}
	}
}