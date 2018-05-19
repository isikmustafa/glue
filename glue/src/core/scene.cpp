#include "scene.h"
#include "pinhole_camera.h"
#include "tonemapper.h"
#include "real_sampler.h"
#include "filter.h"
#include "..\geometry\triangle.h"
#include "..\geometry\transformation.h"
#include "..\geometry\bbox.h"
#include "..\geometry\bvh.h"
#include "..\geometry\mesh.h"
#include "..\light\diffuse_arealight.h"
#include "..\integrator\pathtracer.h"
#include "..\integrator\raytracer.h"
#include "..\material\lambertian.h"
#include "..\material\oren_nayar.h"
#include "..\material\metal.h"
#include "..\material\dielectric.h"

#include <ctpl_stl.h>
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

			if (file.LoadFile(filepath.c_str()))
			{
				throw std::runtime_error("Error: The xml file cannot be loaded!");
			}

			auto scene_element = getFirstChildElementThrow(&file, "Scene");

			parseIntegrator(scene_element);
			parseMeshes(scene_element);
			parseLights(scene_element);
			parseOutput(scene_element);
			parseCamera(scene_element);
			image = std::make_unique<Image>(camera->get_screen_resolution().x, camera->get_screen_resolution().y);

			parseTagContent(scene_element, "BackgroundRadiance", &background_radiance.x, 0.0f, &background_radiance.y, 0.0f, &background_radiance.z, 0.0f);
			parseTagContent(scene_element, "SecondaryRayEpsilon", &secondary_ray_epsilon, 0.0001f);

			debug_bvh.buildWithSAHSplit(debug_spheres);
			bvh.buildWithMedianSplit(meshes);
		}

		void Scene::render()
		{
			constexpr int cPatchSize = 32;
			auto resolution = camera->get_screen_resolution();
			ctpl::thread_pool pool(std::thread::hardware_concurrency());
			for (int x = 0; x < resolution.x; x += cPatchSize)
			{
				for (int y = 0; y < resolution.y; y += cPatchSize)
				{
					pool.push([x, y, cPatchSize, resolution, this](int id)
					{
						for (int i = x; i < x + cPatchSize && i < resolution.x; ++i)
						{
							for (int j = y; j < y + cPatchSize && j < resolution.y; ++j)
							{
								(*this->image)[i][j] = this->m_integrator->integratePixel(*this, i, j);
							}
						}
					});
				}
			}
			pool.stop(true);

			for (const auto& img : output)
			{
				img.first->tonemap(*image).save(img.second);
			}
		}

		void Scene::parseIntegrator(tinyxml2::XMLElement* scene_element)
		{
			auto integrator_element = getFirstChildElementThrow(scene_element, "Integrator");
			auto integrator_type = getAttributeThrow(integrator_element, "type");

			if (integrator_type == std::string("Raytracer"))
			{
				m_integrator = std::make_unique<integrator::Raytracer>();
			}
			else if (integrator_type == std::string("Pathtracer"))
			{
				auto filter_element = getFirstChildElementThrow(integrator_element, "Filter");

				int sample_count;
				parseTagContent(integrator_element, "SampleCount", &sample_count, 1);

				float rr_threshold;
				parseTagContent(integrator_element, "RRThreshold", &rr_threshold, 0.5f);

				m_integrator = std::make_unique<integrator::Pathtracer>(parseFilter(filter_element), sample_count, rr_threshold);
			}
			else
			{
				throw std::runtime_error("Error: Unknown Integrator type");
			}
		}

		void Scene::parseCamera(tinyxml2::XMLElement* scene_element)
		{
			auto camera_element = getFirstChildElementThrow(scene_element, "Camera");

			glm::vec3 position;
			glm::vec3 direction;
			glm::vec3 up;
			glm::vec2 fov_xy;
			glm::ivec2 resolution;
			float near_distance;

			parseTagContent(camera_element, "Position", &position.x, &position.y, &position.z);
			parseTagContent(camera_element, "Direction", &direction.x, &direction.y, &direction.z);
			parseTagContent(camera_element, "Up", &up.x, &up.y, &up.z);
			parseTagContent(camera_element, "FovXY", &fov_xy.x, &fov_xy.y);
			parseTagContent(camera_element, "Resolution", &resolution.x, &resolution.y);
			parseTagContent(camera_element, "NearDistance", &near_distance);

			auto s_right = glm::tan(glm::radians(fov_xy.x) * 0.5f) * near_distance;
			auto s_up = glm::tan(glm::radians(fov_xy.y) * 0.5f) * near_distance;

			camera = std::make_unique<PinholeCamera>(position, direction, up, glm::vec4(-s_right, s_right, -s_up, s_up), resolution, near_distance);
		}

		void Scene::parseOutput(tinyxml2::XMLElement* scene_element)
		{
			auto output_element = getFirstChildElementThrow(scene_element, "Output");

			auto image_element = getFirstChildElementThrow(output_element, "Image");
			while (image_element)
			{
				std::string image_name;
				parseTagContent(image_element, "ImageName", &image_name);

				auto tonemapper_element = getFirstChildElementThrow(image_element, "Tonemapper");
				auto tonemapper_type = getAttributeThrow(tonemapper_element, "type");

				if (tonemapper_type == std::string("Clamp"))
				{
					float min;
					float max;

					parseTagContent(tonemapper_element, "Min", &min);
					parseTagContent(tonemapper_element, "Max", &max);

					output.emplace_back(std::make_unique<Clamp>(min, max), std::move(image_name));
				}
				else if (tonemapper_type == std::string("GlobalReinhard"))
				{
					float key;
					float max_luminance;

					parseTagContent(tonemapper_element, "Key", &key);
					parseTagContent(tonemapper_element, "MaxLuminance", &max_luminance);

					output.emplace_back(std::make_unique<GlobalReinhard>(key, max_luminance), std::move(image_name));
				}
				else
				{
					throw std::runtime_error("Error: Unknown Tonemapper type");
				}

				image_element = image_element->NextSiblingElement("Image");
			}
		}

		void Scene::parseMeshes(tinyxml2::XMLElement* scene_element)
		{
			auto mesh_element = scene_element->FirstChildElement("Mesh");
			while (mesh_element)
			{
				parseMesh(mesh_element);

				mesh_element = mesh_element->NextSiblingElement("Mesh");
			}
		}

		void Scene::parseLights(tinyxml2::XMLElement* scene_element)
		{
			auto light_element = scene_element->FirstChildElement("Light");
			while (light_element)
			{
				auto light_type = getAttributeThrow(light_element, "type");

				if (light_type == std::string("DiffuseArealight"))
				{
					parseMesh(light_element);

					glm::vec3 flux;
					parseTagContent(light_element, "Flux", &flux.x, &flux.y, &flux.z);

					lights.push_back(std::make_shared<light::DiffuseArealight>(meshes.back(), flux));
					light_meshes[meshes.back().get()] = lights.back().get();
				}
				else
				{
					throw std::runtime_error("Error: Unknown Light type");
				}

				light_element = light_element->NextSiblingElement("Light");
			}
		}

		void Scene::parseMesh(tinyxml2::XMLElement* mesh_element)
		{
			std::string datapath;
			parseTagContent(mesh_element, "Datapath", &datapath);

			if (m_path_to_triangles.find(datapath) == m_path_to_triangles.end())
			{
				parseTriangles(datapath);
			}

			geometry::Transformation transformation;
			if (mesh_element->FirstChildElement("Transformation"))
			{
				transformation = parseTransformation(mesh_element->FirstChildElement("Transformation"));
			}

			geometry::BBox bbox;
			std::vector<float> triangle_areas;
			auto total_area = 0.0f;
			for (const auto& triangle : *m_path_to_triangles[datapath])
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

			std::unique_ptr<material::BsdfMaterial> bsdf_material = nullptr;

			if (mesh_element->Value() == std::string("Mesh"))
			{
				auto bsdf_material_element = getFirstChildElementThrow(mesh_element, "BsdfMaterial");
				bsdf_material = parseBsdfMaterial(bsdf_material_element);
			}

			meshes.push_back(std::make_shared<geometry::Mesh>(transformation, bbox, triangle_areas, total_area, m_path_to_triangles[datapath], m_path_to_bvh[datapath],
				std::move(bsdf_material)));

			//Create debug spheres on randomly chosen points if 'displayRandomSamples' attribute is specified.
			auto att = mesh_element->Attribute("displayRandomSamples");
			if (att)
			{
				UniformSampler sampler;
				const auto& mesh = meshes.back();
				auto num_of_samples = std::atoi(att);
				for (int i = 0; i < num_of_samples; ++i)
				{
					debug_spheres.emplace_back(mesh->samplePlane(sampler).point, glm::sqrt(mesh->getSurfaceArea() / num_of_samples) / 5.0f);
				}
			}
		}

		void Scene::parseTriangles(const std::string& datapath)
		{
			std::vector<geometry::Triangle> triangles;

			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(datapath,
				aiProcess_Triangulate |
				aiProcess_JoinIdenticalVertices);

			if (!scene)
			{
				throw std::runtime_error("Error: Assimp cannot load the model!");
			}

			if (scene->mNumMeshes > 1)
			{
				throw std::runtime_error("Unhandled case: More than one mesh to process in the same model!");
			}

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
				triangles.emplace_back(face_vertices[0], face_vertices[1] - face_vertices[0], face_vertices[2] - face_vertices[0]);
			}

			geometry::BVH bvh;
			bvh.buildWithSAHSplit(triangles);

			m_path_to_triangles.insert({ datapath, std::make_shared<std::vector<geometry::Triangle>>(std::move(triangles)) });
			m_path_to_bvh.insert({ datapath, std::make_shared<geometry::BVH>(std::move(bvh)) });
		}

		std::unique_ptr<core::Filter> Scene::parseFilter(tinyxml2::XMLElement* filter_element)
		{
			auto filter_type = getAttributeThrow(filter_element, "type");

			if (filter_type == std::string("Box"))
			{
				return std::make_unique<BoxFilter>();
			}
			else if (filter_type == std::string("Gaussian"))
			{
				float sigma;

				parseTagContent(filter_element, "Sigma", &sigma, 0.5f);

				return std::make_unique<GaussianFilter>(sigma);
			}
			else
			{
				throw std::runtime_error("Error: Unknown Filter type");
			}
		}

		geometry::Transformation Scene::parseTransformation(tinyxml2::XMLElement* transformation_element)
		{
			geometry::Transformation transformation;

			glm::vec3 scaling;
			parseTagContent(transformation_element, "Scaling", &scaling.x, 1.0f, &scaling.y, 1.0f, &scaling.z, 1.0f);
			transformation.scale(scaling);

			glm::vec3 axis;
			float angle;
			parseTagContent(transformation_element, "Rotation", &axis.x, 1.0f, &axis.y, 1.0f, &axis.z, 1.0f, &angle, 0.0f);
			transformation.rotate(glm::normalize(axis), angle);

			glm::vec3 translation;
			parseTagContent(transformation_element, "Translation", &translation.x, 0.0f, &translation.y, 0.0f, &translation.z, 0.0f);
			transformation.translate(translation);

			return transformation;
		}

		std::unique_ptr<material::BsdfMaterial> Scene::parseBsdfMaterial(tinyxml2::XMLElement* bsdf_material_element)
		{
			auto bsdf_type = getAttributeThrow(bsdf_material_element, "type");

			if (bsdf_type == std::string("Lambertian"))
			{
				glm::vec3 kd;

				parseTagContent(bsdf_material_element, "kd", &kd.x, &kd.y, &kd.z);

				return std::make_unique<material::Lambertian>(kd);
			}
			else if (bsdf_type == std::string("OrenNayar"))
			{
				glm::vec3 kd;
				float roughness;

				parseTagContent(bsdf_material_element, "kd", &kd.x, &kd.y, &kd.z);
				parseTagContent(bsdf_material_element, "Roughness", &roughness);

				return std::make_unique<material::OrenNayar>(kd, roughness);
			}
			else if (bsdf_type == std::string("Metal"))
			{
				glm::vec3 ior_n;
				glm::vec3 ior_k;
				float roughness;

				parseTagContent(bsdf_material_element, "IorN", &ior_n.x, &ior_n.y, &ior_n.z);
				parseTagContent(bsdf_material_element, "IorK", &ior_k.x, &ior_k.y, &ior_k.z);
				parseTagContent(bsdf_material_element, "Roughness", &roughness);

				return std::make_unique<material::Metal>(ior_n, ior_k, roughness);
			}
			else if (bsdf_type == std::string("Dielectric"))
			{
				float ior_n;
				float roughness;

				parseTagContent(bsdf_material_element, "IorN", &ior_n);
				parseTagContent(bsdf_material_element, "Roughness", &roughness);

				return std::make_unique<material::Dielectric>(ior_n, roughness);
			}
			else
			{
				throw std::runtime_error("Error: Unknown BsdfMaterial type");
			}
		}
	}
}