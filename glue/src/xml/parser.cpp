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
			static std::unordered_map<std::string, std::shared_ptr<geometry::BVH<geometry::Triangle>>> gPathToBvh;

			if (gPathToBvh.find(path) == gPathToBvh.end())
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

				auto& bvh = gPathToBvh[path] = std::make_shared<geometry::BVH<geometry::Triangle>>();

				aiMesh* mesh = scene->mMeshes[0];
				glm::vec3 face_vertices[3];
				int face_count = mesh->mNumFaces;
				for (int i = 0; i < face_count; ++i)
				{
					const aiFace& face = mesh->mFaces[i];
					for (int k = 0; k < 3; ++k)
					{
						const aiVector3D& position = mesh->mVertices[face.mIndices[k]];
						face_vertices[k] = glm::vec3(position.x, position.y, position.z);
					}
					bvh->addObject(geometry::Triangle(face_vertices[0], face_vertices[1] - face_vertices[0], face_vertices[2] - face_vertices[0]));
				}

				bvh->buildWithSAHSplit();
			}

			return gPathToBvh[path];
		}
	}
}