#include "parser.h"
#include "..\geometry\bvh.h"
#include "..\material\lambertian.h"

#include <sstream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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
					auto child = camera_element->FirstChildElement("ScreenCoordinates");
					stream << child->GetText() << std::endl;
					child = camera_element->FirstChildElement("Position");
					stream << child->GetText() << std::endl;
					child = camera_element->FirstChildElement("Direction");
					stream << child->GetText() << std::endl;
					child = camera_element->FirstChildElement("Up");
					stream << child->GetText() << std::endl;
					child = camera_element->FirstChildElement("Resolution");
					stream << child->GetText() << std::endl;
					child = camera_element->FirstChildElement("NearDistance");
					stream << child->GetText() << std::endl;

					glm::vec4 screen_coordinates;
					glm::vec3 position;
					glm::vec3 direction;
					glm::vec3 up;
					glm::ivec2 screen_resolution;
					float near_distance;

					stream >> screen_coordinates.x >> screen_coordinates.y >> screen_coordinates.z >> screen_coordinates.w;
					stream >> position.x >> position.y >> position.z;
					stream >> direction.x >> direction.y >> direction.z;
					stream >> up.x >> up.y >> up.z;
					stream >> screen_resolution.x >> screen_resolution.y;
					stream >> near_distance;

					return std::make_unique<PinholeCamera>(position, direction, up, screen_coordinates, screen_resolution, near_distance);
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
					aiProcess_JoinIdenticalVertices |
					aiProcess_GenSmoothNormals);

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
					auto bsdf_material = bsdf_material_element->FirstChildElement();
					auto element_value = bsdf_material->Value();

					if (element_value == std::string("Lambertian"))
					{
						glm::vec3 kd;
						stream << bsdf_material->FirstChildElement("kd")->GetText();
						stream >> kd.x >> kd.y >> kd.z;

						return std::make_unique<material::Lambertian>(kd);
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
}