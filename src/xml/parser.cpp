#include "parser.h"
#include "../geometry/triangle.h"
#include <iostream>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

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
				tinyobj::attrib_t attrib;
				std::vector<tinyobj::shape_t> shapes;
				std::vector<tinyobj::material_t> materials;
				std::string err;
                std::string warn;

                auto res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), nullptr, true, false);
                //std::cout << warn << std::endl;
				if (!res)
				{
                    std::cout << err << std::endl;
					throw std::runtime_error("Error: Model cannot loaded.");
				}

				auto& bvh = path_to_bvh[path] = std::make_shared<geometry::BVH<geometry::Triangle>>();

				int shapes_size = shapes.size();
				if (shapes_size > 1)
				{
					std::cout << "Warning: Model contains more than one shape." << std::endl;
				}

				for (int s = 0; s < shapes_size; ++s)
				{
					int index_offset = 0;
					glm::vec3 face_vertices[3];
					glm::vec2 tex_coords[3]{ glm::vec2(0.0f), glm::vec2(0.0f), glm::vec2(0.0f) };
					int num_face_vertices_size = shapes[s].mesh.num_face_vertices.size();
					for (int f = 0; f < num_face_vertices_size; ++f)
					{
						for (int v = 0; v < 3; ++v)
						{
							tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

							int v_idx = 3 * idx.vertex_index;
							face_vertices[v].x = attrib.vertices[v_idx++];
							face_vertices[v].y = attrib.vertices[v_idx++];
							face_vertices[v].z = attrib.vertices[v_idx++];
                            if (idx.texcoord_index != -1)
                            {
                                int t_idx = 2 * idx.texcoord_index;
                                tex_coords[v].x = attrib.texcoords[t_idx++];
                                tex_coords[v].y = attrib.texcoords[t_idx++];
                            }
						}
						index_offset += 3;

						bvh->addObject(geometry::Triangle(face_vertices[0], face_vertices[1] - face_vertices[0], face_vertices[2] - face_vertices[0],
														  tex_coords[0], tex_coords[1], tex_coords[2]));
					}
				}

				bvh->buildWithSAHSplit();
			}

			return path_to_bvh[path];
		}

		std::shared_ptr<std::vector<core::Image>> Parser::loadImage(const std::string& path, bool mipmapping)
		{
			static std::unordered_map<std::string, std::shared_ptr<std::vector<core::Image>>> path_to_image;

			if (path_to_image.find(path) == path_to_image.end())
			{
				core::Image image(path);

				if (mipmapping)
				{
					path_to_image[path] = std::make_shared<std::vector<core::Image>>(image.generateMipmaps());
				}
				else
				{
					path_to_image[path] = std::make_shared<std::vector<core::Image>>(1, std::move(image));
				}
			}

			return path_to_image[path];
		}
	}
}