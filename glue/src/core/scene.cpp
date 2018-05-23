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
		void Scene::load(const std::string& xml_filepath)
		{
			auto root = xml::Node::getRoot(xml_filepath);

			parseCamera(root);
			parseLights(root);
			root.parseChildText("BackgroundRadiance", &background_radiance.x, 0.0f, &background_radiance.y, 0.0f, &background_radiance.z, 0.0f);
			root.parseChildText("SecondaryRayEpsilon", &secondary_ray_epsilon, 0.0001f);

			parseIntegrator(root);
			m_image = std::make_unique<Image>(camera->get_screen_resolution().x, camera->get_screen_resolution().y);
			parseOutput(root);
			parseObjects(root);

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

		void Scene::parseIntegrator(const xml::Node& root)
		{
			auto integrator_element = root.child("Integrator", true);
			auto integrator_type = integrator_element.attribute("type", true);

			if (integrator_type == std::string("Pathtracer"))
			{
				int sample_count;
				float rr_threshold;

				integrator_element.parseChildText("SampleCount", &sample_count, 1);
				integrator_element.parseChildText("RRThreshold", &rr_threshold, 0.5f);

				auto filter_element = integrator_element.child("Filter", true);
				m_integrator = std::make_unique<integrator::Pathtracer>(parseFilter(filter_element), sample_count, rr_threshold);
			}
			else
			{
				integrator_element.throwError("Unknown Integrator type.");
			}
		}

		void Scene::parseCamera(const xml::Node& root)
		{
			auto camera_element = root.child("Camera", true);

			glm::vec3 position;
			glm::vec3 direction;
			glm::vec3 up;
			glm::vec2 fov_xy;
			glm::ivec2 resolution;
			float near_distance;

			camera_element.parseChildText("Position", &position.x, &position.y, &position.z);
			camera_element.parseChildText("Direction", &direction.x, &direction.y, &direction.z);
			camera_element.parseChildText("Up", &up.x, &up.y, &up.z);
			camera_element.parseChildText("FovXY", &fov_xy.x, &fov_xy.y);
			camera_element.parseChildText("Resolution", &resolution.x, &resolution.y);
			camera_element.parseChildText("NearDistance", &near_distance);

			auto s_right = glm::tan(glm::radians(fov_xy.x) * 0.5f) * near_distance;
			auto s_up = glm::tan(glm::radians(fov_xy.y) * 0.5f) * near_distance;

			camera = std::make_unique<PinholeCamera>(position, direction, up, glm::vec4(-s_right, s_right, -s_up, s_up), resolution, near_distance);
		}

		void Scene::parseOutput(const xml::Node& root)
		{
			auto output_element = root.child("Output", true);
			while (output_element)
			{
				auto output_type = output_element.attribute("type", true);

				if (output_type == std::string("Ldr"))
				{
					std::string image_name;
					std::string image_format;

					output_element.parseChildText("ImageName", &image_name);
					output_element.parseChildText("ImageFormat", &image_format, std::string("png"));

					if (m_supported_imageformats_save.find(image_format) == m_supported_imageformats_save.end())
					{
						output_element.child("ImageFormat").throwError("Unsupported ImageFormat.");
					}

					image_name += "." + image_format;

					auto tonemapper_element = output_element.child("Tonemapper", true);
					auto tonemapper_type = tonemapper_element.attribute("type", true);

					if (tonemapper_type == std::string("Clamp"))
					{
						float min;
						float max;

						tonemapper_element.parseChildText("Min", &min);
						tonemapper_element.parseChildText("Max", &max);

						m_outputs.push_back(std::make_unique<Ldr>(std::move(image_name), std::make_unique<Clamp>(min, max)));
					}
					else if (tonemapper_type == std::string("GlobalReinhard"))
					{
						float key;
						float max_luminance;

						tonemapper_element.parseChildText("Key", &key);
						tonemapper_element.parseChildText("MaxLuminance", &max_luminance);

						m_outputs.push_back(std::make_unique<Ldr>(std::move(image_name), std::make_unique<GlobalReinhard>(key, max_luminance)));
					}
					else
					{
						tonemapper_element.throwError("Unknown Tonemapper type.");
					}
				}
				else
				{
					output_element.throwError("Unknown Output type.");
				}

				output_element = output_element.next();
			}
		}

		void Scene::parseObjects(const xml::Node& root)
		{
			auto object_element = root.child("Object");
			while (object_element)
			{
				parseObject(object_element);

				object_element = object_element.next();
			}
		}

		void Scene::parseLights(const xml::Node& root)
		{
			auto light_element = root.child("Light");
			while (light_element)
			{
				auto light_type = light_element.attribute("type", true);

				if (light_type == std::string("DiffuseArealight"))
				{
					auto object_element = light_element.child("Object", true);
					auto object = parseObject(object_element);

					glm::vec3 flux;
					light_element.parseChildText("Flux", &flux.x, &flux.y, &flux.z);

					lights.push_back(std::make_unique<light::DiffuseArealight>(object, flux));
					light_meshes[object.get()] = lights.back().get();
				}
				else
				{
					light_element.throwError("Unknown Light type.");
				}

				light_element = light_element.next();
			}
		}

		std::shared_ptr<geometry::Object> Scene::parseObject(const xml::Node& object)
		{
			auto object_type = object.attribute("type", true);

			if (object_type == std::string("Mesh"))
			{
				parseMesh(object);
				return m_bvh_meshes.get_objects().back();
			}
			else if (object_type == std::string("Sphere"))
			{
				parseSphere(object);
				return m_bvh_spheres.get_objects().back();
			}
			else
			{
				object.throwError("Unknown Object type.");
			}
		}

		void Scene::parseMesh(const xml::Node& mesh)
		{
			std::string datapath;
			mesh.parseChildText("Datapath", &datapath);

			if (m_path_to_bvh.find(datapath) == m_path_to_bvh.end())
			{
				parseTriangles(datapath);
			}

			geometry::Transformation transformation;
			auto transformation_element = mesh.child("Transformation");
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

			if (mesh.parent().value() != std::string("Light"))
			{
				auto bsdf_material_element = mesh.child("BsdfMaterial", true);
				bsdf_material = parseBsdfMaterial(bsdf_material_element);
			}

			m_bvh_meshes.addObject(std::make_shared<geometry::Mesh>(transformation, bbox, triangle_areas, total_area, m_path_to_bvh[datapath],
				std::move(bsdf_material)));
		}

		void Scene::parseSphere(const xml::Node& sphere)
		{
			float radius;
			glm::vec3 center;

			sphere.parseChildText("Radius", &radius);
			sphere.parseChildText("Center", &center.x, &center.y, &center.z);

			geometry::Transformation transformation;
			auto transformation_element = sphere.child("Transformation");
			if (transformation_element)
			{
				glm::vec3 scaling;
				glm::vec3 axis;
				float angle;
				glm::vec3 translation;

				transformation_element.parseChildText("Scaling", &scaling.x, 1.0f, &scaling.y, 1.0f, &scaling.z, 1.0f);
				transformation_element.parseChildText("Rotation", &axis.x, 1.0f, &axis.y, 1.0f, &axis.z, 1.0f, &angle, 0.0f);
				transformation_element.parseChildText("Translation", &translation.x, 0.0f, &translation.y, 0.0f, &translation.z, 0.0f);

				if (!(scaling.x == scaling.y && scaling.y == scaling.z))
				{
					transformation_element.child("Scaling").throwError("Non-uniform scaling is not allowed for spheres.");
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

			if (sphere.parent().value() != std::string("Light"))
			{
				auto bsdf_material_element = sphere.child("BsdfMaterial", true);
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

		std::unique_ptr<core::Filter> Scene::parseFilter(const xml::Node& filter)
		{
			auto filter_type = filter.attribute("type", true);

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
				filter.parseChildText("Sigma", &sigma, 0.5f);

				return std::make_unique<GaussianFilter>(sigma);
			}
			else
			{
				filter.throwError("Unknown Filter type.");
			}
		}

		geometry::Transformation Scene::parseTransformation(const xml::Node& transformation)
		{
			glm::vec3 scaling;
			glm::vec3 axis;
			float angle;
			glm::vec3 translation;

			transformation.parseChildText("Scaling", &scaling.x, 1.0f, &scaling.y, 1.0f, &scaling.z, 1.0f);
			transformation.parseChildText("Rotation", &axis.x, 1.0f, &axis.y, 1.0f, &axis.z, 1.0f, &angle, 0.0f);
			transformation.parseChildText("Translation", &translation.x, 0.0f, &translation.y, 0.0f, &translation.z, 0.0f);

			geometry::Transformation t;
			t.scale(scaling);
			t.rotate(glm::normalize(axis), angle);
			t.translate(translation);

			return t;
		}

		std::unique_ptr<material::BsdfMaterial> Scene::parseBsdfMaterial(const xml::Node& bsdf_material)
		{
			auto bsdf_type = bsdf_material.attribute("type", true);

			if (bsdf_type == std::string("Lambertian"))
			{
				glm::vec3 kd;
				bsdf_material.parseChildText("kd", &kd.x, &kd.y, &kd.z);

				return std::make_unique<material::Lambertian>(kd);
			}
			else if (bsdf_type == std::string("OrenNayar"))
			{
				glm::vec3 kd;
				float roughness;

				bsdf_material.parseChildText("kd", &kd.x, &kd.y, &kd.z);
				bsdf_material.parseChildText("Roughness", &roughness);

				return std::make_unique<material::OrenNayar>(kd, roughness);
			}
			else if (bsdf_type == std::string("Metal"))
			{
				glm::vec3 ior_n;
				glm::vec3 ior_k;
				float roughness;

				bsdf_material.parseChildText("IorN", &ior_n.x, &ior_n.y, &ior_n.z);
				bsdf_material.parseChildText("IorK", &ior_k.x, &ior_k.y, &ior_k.z);
				bsdf_material.parseChildText("Roughness", &roughness);

				return std::make_unique<material::Metal>(ior_n, ior_k, roughness);
			}
			else if (bsdf_type == std::string("Dielectric"))
			{
				float ior_n;
				float roughness;

				bsdf_material.parseChildText("IorN", &ior_n);
				bsdf_material.parseChildText("Roughness", &roughness);

				return std::make_unique<material::Dielectric>(ior_n, roughness);
			}
			else
			{
				bsdf_material.throwError("Unknown BsdfMaterial type.");
			}
		}
	}
}