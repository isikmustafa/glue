#include "parser.h"
#include "..\geometry\triangle.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

namespace glue
{
	namespace xml
	{
		const std::unordered_set<std::string> Parser::gSupportedFormatsLoad{ "jpg", "png", "tga", "bmp", "psd", "gif", "hdr", "pic" };
		const std::unordered_set<std::string> Parser::gSupportedFormatsSave{ "png", "bmp", "tga" };

		core::Scene::Xml Parser::parse(const std::string& xml_filepath)
		{
			return core::Scene::Xml(glue::xml::Node::getRoot(xml_filepath));
		}

		std::shared_ptr<geometry::BVH<geometry::Triangle>> Parser::loadModel(const std::string& path)
		{
			static std::unordered_map<std::string, std::shared_ptr<geometry::BVH<geometry::Triangle>>> path_to_bvh;

			if (path_to_bvh.find(path) == path_to_bvh.end())
			{
				Assimp::Importer importer;
				const aiScene* scene = importer.ReadFile(path,
					aiProcess_Triangulate |
					aiProcess_JoinIdenticalVertices);

				if (!scene)
				{
					throw std::runtime_error("Error: Assimp cannot load the model");
				}

				if (scene->mNumMeshes > 1)
				{
					throw std::runtime_error("Unhandled case: More than one mesh to process in the same model");
				}

				auto& bvh = path_to_bvh[path] = std::make_shared<geometry::BVH<geometry::Triangle>>();

				aiMesh* mesh = scene->mMeshes[0];
				glm::vec3 face_vertices[3];
				glm::vec2 tex_coords[3] = { glm::vec2(0.0f), glm::vec2(0.0f), glm::vec2(0.0f) };
				int face_count = mesh->mNumFaces;
				for (int i = 0; i < face_count; ++i)
				{
					const aiFace& face = mesh->mFaces[i];
					for (int k = 0; k < 3; ++k)
					{
						const aiVector3D& position = mesh->mVertices[face.mIndices[k]];
						face_vertices[k] = glm::vec3(position.x, position.y, position.z);

						if (mesh->HasTextureCoords(0))
						{
							const aiVector3D& uv = mesh->mTextureCoords[0][face.mIndices[k]];
							tex_coords[k] = glm::vec2(uv.x, uv.y);
						}
					}
					bvh->addObject(geometry::Triangle(face_vertices[0], face_vertices[1] - face_vertices[0], face_vertices[2] - face_vertices[0], 
						tex_coords[0], tex_coords[1], tex_coords[2]));
				}

				bvh->buildWithSAHSplit();
			}

			return path_to_bvh[path];
		}

		std::shared_ptr<std::vector<core::Image>> Parser::loadImage(const std::string& path)
		{
			static std::unordered_map<std::string, std::shared_ptr<std::vector<core::Image>>> path_to_image;

			if (path_to_image.find(path) == path_to_image.end())
			{
				core::Image image(path);

				auto& mipmaps = path_to_image[path] = std::make_shared<std::vector<core::Image>>(image.generateMipmaps());
				mipmaps->insert(mipmaps->begin(), std::move(image));
			}

			return path_to_image[path];
		}
	}
}