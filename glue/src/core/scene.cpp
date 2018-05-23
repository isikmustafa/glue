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
#include "..\material\lambertian.h"
#include "..\material\oren_nayar.h"
#include "..\material\metal.h"
#include "..\material\dielectric.h"

#include <ctpl_stl.h>
#include <utility>
#include <glm\vec2.hpp>
#include <glm\vec3.hpp>
#include <glm\trigonometric.hpp>
#include <glm\gtc\constants.hpp>
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
				throwXMLError(&file, "The xml file cannot be loaded.");
			}

			auto scene_element = getFirstChildElementThrow(&file, "Scene");

			parseCamera(scene_element);
			parseLights(scene_element);
			parseTagContent(scene_element, "BackgroundRadiance", &background_radiance.x, 0.0f, &background_radiance.y, 0.0f, &background_radiance.z, 0.0f);
			parseTagContent(scene_element, "SecondaryRayEpsilon", &secondary_ray_epsilon, 0.0001f);

			parseIntegrator(scene_element);
			m_image = std::make_unique<Image>(camera->get_screen_resolution().x, camera->get_screen_resolution().y);
			parseOutput(scene_element);
			parseObjects(scene_element);

			m_bvh_meshes.buildWithMedianSplit();
			m_bvh_spheres.buildWithSAHSplit();
		}

		bool Scene::intersect(const geometry::Ray& ray, geometry::Intersection& intersection, float max_distance) const
		{
			auto result_meshes = m_bvh_meshes.intersect(ray, intersection, max_distance);
			auto result_spheres = m_bvh_spheres.intersect(ray, intersection, result_meshes ? intersection.distance : max_distance);

			return result_meshes || result_spheres;
		}

		bool Scene::intersectShadowRay(const geometry::Ray& ray, float max_distance) const
		{
			return m_bvh_meshes.intersectShadowRay(ray, max_distance) || m_bvh_spheres.intersectShadowRay(ray, max_distance);
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
								(*this->m_image)[i][j] = this->m_integrator->integratePixel(*this, i, j);
							}
						}
					});
				}
			}
			pool.stop(true);

			for (const auto& output : m_outputs)
			{
				output->save(*m_image);
			}
		}

		void Scene::parseIntegrator(tinyxml2::XMLElement* scene_element)
		{
			auto integrator_element = getFirstChildElementThrow(scene_element, "Integrator");
			auto integrator_type = getAttributeThrow(integrator_element, "type");

			if (integrator_type == std::string("Pathtracer"))
			{
				int sample_count;
				float rr_threshold;

				parseTagContent(integrator_element, "SampleCount", &sample_count, 1);
				parseTagContent(integrator_element, "RRThreshold", &rr_threshold, 0.5f);

				auto filter_element = getFirstChildElementThrow(integrator_element, "Filter");
				m_integrator = std::make_unique<integrator::Pathtracer>(parseFilter(filter_element), sample_count, rr_threshold);
			}
			else
			{
				throwXMLError(integrator_element, "Unknown Integrator type.");
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
			while (output_element)
			{
				auto output_type = getAttributeThrow(output_element, "type");

				if (output_type == std::string("Ldr"))
				{
					std::string image_name;
					std::string image_format;

					parseTagContent(output_element, "ImageName", &image_name);
					parseTagContent(output_element, "ImageFormat", &image_format, std::string("png"));

					if (m_supported_imageformats_save.find(image_format) == m_supported_imageformats_save.end())
					{
						throwXMLError(output_element->FirstChildElement("ImageFormat"), "Unsupported ImageFormat.");
					}

					image_name += "." + image_format;

					auto tonemapper_element = getFirstChildElementThrow(output_element, "Tonemapper");
					auto tonemapper_type = getAttributeThrow(tonemapper_element, "type");

					if (tonemapper_type == std::string("Clamp"))
					{
						float min;
						float max;

						parseTagContent(tonemapper_element, "Min", &min);
						parseTagContent(tonemapper_element, "Max", &max);

						m_outputs.push_back(std::make_unique<Ldr>(std::move(image_name), std::make_unique<Clamp>(min, max)));
					}
					else if (tonemapper_type == std::string("GlobalReinhard"))
					{
						float key;
						float max_luminance;

						parseTagContent(tonemapper_element, "Key", &key);
						parseTagContent(tonemapper_element, "MaxLuminance", &max_luminance);

						m_outputs.push_back(std::make_unique<Ldr>(std::move(image_name), std::make_unique<GlobalReinhard>(key, max_luminance)));
					}
					else
					{
						throwXMLError(tonemapper_element, "Unknown Tonemapper type.");
					}
				}
				else
				{
					throwXMLError(output_element, "Unknown Output type.");
				}

				output_element = output_element->NextSiblingElement("Output");
			}
		}

		void Scene::parseObjects(tinyxml2::XMLElement* scene_element)
		{
			auto object_element = scene_element->FirstChildElement("Object");
			while (object_element)
			{
				parseObject(object_element);

				object_element = object_element->NextSiblingElement("Object");
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
					auto object_element = getFirstChildElementThrow(light_element, "Object");
					auto object = parseObject(object_element);

					glm::vec3 flux;
					parseTagContent(light_element, "Flux", &flux.x, &flux.y, &flux.z);

					lights.push_back(std::make_unique<light::DiffuseArealight>(object, flux));
					light_meshes[object.get()] = lights.back().get();
				}
				else
				{
					throwXMLError(light_element, "Unknown Light type.");
				}

				light_element = light_element->NextSiblingElement("Light");
			}
		}

		std::shared_ptr<geometry::Object> Scene::parseObject(tinyxml2::XMLElement* object_element)
		{
			auto object_type = getAttributeThrow(object_element, "type");

			if (object_type == std::string("Mesh"))
			{
				parseMesh(object_element);
				return m_bvh_meshes.get_objects().back();
			}
			else if (object_type == std::string("Sphere"))
			{
				parseSphere(object_element);
				return m_bvh_spheres.get_objects().back();
			}
			else
			{
				throwXMLError(object_element, "Unknown Object type.");
			}
		}

		void Scene::parseMesh(tinyxml2::XMLElement* mesh_element)
		{
			std::string datapath;
			parseTagContent(mesh_element, "Datapath", &datapath);

			if (m_path_to_bvh.find(datapath) == m_path_to_bvh.end())
			{
				parseTriangles(datapath);
			}

			geometry::Transformation transformation;
			auto transformation_element = mesh_element->FirstChildElement("Transformation");
			if (transformation_element)
			{
				transformation = parseTransformation(transformation_element);
			}

			geometry::BBox bbox;
			std::vector<float> triangle_areas;
			auto total_area = 0.0f;
			for (const auto& triangle : m_path_to_bvh[datapath]->get_objects())
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

			if (mesh_element->Parent()->Value() != std::string("Light"))
			{
				auto bsdf_material_element = getFirstChildElementThrow(mesh_element, "BsdfMaterial");
				bsdf_material = parseBsdfMaterial(bsdf_material_element);
			}

			m_bvh_meshes.addObject(std::make_shared<geometry::Mesh>(transformation, bbox, triangle_areas, total_area, m_path_to_bvh[datapath],
				std::move(bsdf_material)));
		}

		void Scene::parseSphere(tinyxml2::XMLElement* sphere_element)
		{
			float radius;
			glm::vec3 center;

			parseTagContent(sphere_element, "Radius", &radius);
			parseTagContent(sphere_element, "Center", &center.x, &center.y, &center.z);

			geometry::Transformation transformation;
			auto transformation_element = sphere_element->FirstChildElement("Transformation");
			if (transformation_element)
			{
				glm::vec3 scaling;
				glm::vec3 axis;
				float angle;
				glm::vec3 translation;

				parseTagContent(transformation_element, "Scaling", &scaling.x, 1.0f, &scaling.y, 1.0f, &scaling.z, 1.0f);
				parseTagContent(transformation_element, "Rotation", &axis.x, 1.0f, &axis.y, 1.0f, &axis.z, 1.0f, &angle, 0.0f);
				parseTagContent(transformation_element, "Translation", &translation.x, 0.0f, &translation.y, 0.0f, &translation.z, 0.0f);

				if (!(scaling.x == scaling.y && scaling.y == scaling.z))
				{
					throwXMLError(transformation_element->FirstChildElement("Scaling"), "Non-uniform scaling is not allowed for spheres.");
				}

				transformation.rotate(glm::normalize(axis), angle);
				radius *= scaling.x;
				center += translation;
			}
			transformation.scale(glm::vec3(radius));
			transformation.translate(center);

			geometry::BBox bbox(center - glm::vec3(radius), center + glm::vec3(radius));
			auto area = 4.0f * glm::pi<float>() * radius * radius;
			std::unique_ptr<material::BsdfMaterial> bsdf_material = nullptr;

			if (sphere_element->Parent()->Value() != std::string("Light"))
			{
				auto bsdf_material_element = getFirstChildElementThrow(sphere_element, "BsdfMaterial");
				bsdf_material = parseBsdfMaterial(bsdf_material_element);
			}

			m_bvh_spheres.addObject(std::make_shared<geometry::Sphere>(transformation, bbox, area, std::move(bsdf_material)));
		}

		void Scene::parseTriangles(const std::string& datapath)
		{
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(datapath,
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

			auto& bvh = m_path_to_bvh[datapath] = std::make_shared<geometry::BVH<geometry::Triangle>>();

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

		std::unique_ptr<core::Filter> Scene::parseFilter(tinyxml2::XMLElement* filter_element)
		{
			auto filter_type = getAttributeThrow(filter_element, "type");

			if (filter_type == std::string("Box"))
			{
				return std::make_unique<BoxFilter>();
			}
			else if (filter_type == std::string("Tent"))
			{
				return std::make_unique<TentFilter>();
			}
			else if (filter_type == std::string("Gaussian"))
			{
				float sigma;
				parseTagContent(filter_element, "Sigma", &sigma, 0.5f);

				return std::make_unique<GaussianFilter>(sigma);
			}
			else
			{
				throwXMLError(filter_element, "Unknown Filter type.");
			}
		}

		geometry::Transformation Scene::parseTransformation(tinyxml2::XMLElement* transformation_element)
		{
			geometry::Transformation transformation;

			glm::vec3 scaling;
			glm::vec3 axis;
			float angle;
			glm::vec3 translation;

			parseTagContent(transformation_element, "Scaling", &scaling.x, 1.0f, &scaling.y, 1.0f, &scaling.z, 1.0f);
			parseTagContent(transformation_element, "Rotation", &axis.x, 1.0f, &axis.y, 1.0f, &axis.z, 1.0f, &angle, 0.0f);
			parseTagContent(transformation_element, "Translation", &translation.x, 0.0f, &translation.y, 0.0f, &translation.z, 0.0f);

			transformation.scale(scaling);
			transformation.rotate(glm::normalize(axis), angle);
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
				throwXMLError(bsdf_material_element, "Unknown BsdfMaterial type.");
			}
		}
	}
}