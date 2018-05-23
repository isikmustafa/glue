#ifndef __GLUE__CORE__SCENE__
#define __GLUE__CORE__SCENE__

#include "pinhole_camera.h"
#include "image.h"
#include "tonemapper.h"
#include "output.h"
#include "..\geometry\mesh.h"
#include "..\geometry\bvh.h"
#include "..\geometry\sphere.h"
#include "..\light\light.h"
#include "..\xml\node.h"

#include <vector>
#include <sstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <glm\vec3.hpp>
#include <tinyxml2.h>

namespace glue
{
	namespace core
	{
		struct Scene
		{
		public:
			std::unique_ptr<PinholeCamera> camera;
			std::vector<std::unique_ptr<light::Light>> lights;
			std::unordered_map<const geometry::Object*, const light::Light*> light_meshes;
			glm::vec3 background_radiance;
			float secondary_ray_epsilon;

		public:
			void load(const std::string& xml_filepath);
			bool intersect(const geometry::Ray& ray, geometry::Intersection& intersection, float max_distance) const;
			bool intersectShadowRay(const geometry::Ray& ray, float max_distance) const;
			void render();

		private:
			std::unique_ptr<integrator::Integrator> m_integrator;
			std::unique_ptr<Image> m_image;
			std::vector<std::unique_ptr<Output>> m_outputs;
			geometry::BVH<std::shared_ptr<geometry::Mesh>> m_bvh_meshes;
			geometry::BVH<std::shared_ptr<geometry::Sphere>> m_bvh_spheres;
			std::unordered_map<std::string, std::shared_ptr<geometry::BVH<geometry::Triangle>>> m_path_to_bvh;
			std::unordered_set<std::string> m_supported_imageformats_load{ "jpg", "png", "tga", "bmp", "psd", "gif", "hdr", "pic" };
			std::unordered_set<std::string> m_supported_imageformats_save{ "png", "bmp", "tga" };
			std::stringstream m_stream;

		private:
			//Caller always makes sure that XMLElement pointer is not nullptr.
			//Callee just uses XMLElement pointer and does not check it. 
			void parseIntegrator(const xml::Node& root);
			void parseCamera(const xml::Node& root);
			void parseOutput(const xml::Node& root);
			void parseObjects(const xml::Node& root);
			void parseLights(const xml::Node& root);
			std::shared_ptr<geometry::Object> parseObject(const xml::Node& object);
			void parseMesh(const xml::Node& mesh);
			void parseSphere(const xml::Node& sphere);
			void parseTriangles(const std::string& datapath);
			std::unique_ptr<core::Filter> parseFilter(const xml::Node& filter);
			geometry::Transformation parseTransformation(const xml::Node& transformation);
			std::unique_ptr<material::BsdfMaterial> parseBsdfMaterial(const xml::Node& bsdf_material);
		};
	}
}

#endif