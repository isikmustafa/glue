#ifndef __GLUE__CORE__SCENE__
#define __GLUE__CORE__SCENE__

#include "pinhole_camera.h"
#include "..\geometry\mesh.h"
#include "..\geometry\bvh.h"
#include "..\geometry\sphere.h"
#include "..\light\light.h"
#include "image.h"
#include "tonemapper.h"
#include "output.h"

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
			void loadFromXML(const std::string& filepath);
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
			//If default values are NOT provided, these two will be called.
			template<typename T>
			void parseTag(T* arg);
			template<typename T, typename... Args>
			void parseTag(T* arg, Args... args);

			//If default values are provided, these two will be called.
			template<typename T>
			void parseTag(T* arg, T default_value);
			template<typename T, typename... Args>
			void parseTag(T* arg, T default_value, Args... args);

			//Parses tag content and assigns them to given arguments.
			template<typename... Args>
			void parseTagContent(tinyxml2::XMLElement* element, const std::string& tag, Args... args);

			//Gets first child element and throws if it does not exist.
			template<typename T>
			tinyxml2::XMLElement* getFirstChildElementThrow(T element, const std::string& tag_name);

			//Gets attribute and throws if it does not exist.
			template<typename T>
			const char* getAttributeThrow(T element, const std::string& att_name);

			//Directly throws a std::runtime_error with given message.
			template<typename T>
			void throwXMLError(T element, const std::string& message);

			//Caller always makes sure that XMLElement pointer is not nullptr.
			//Callee just uses XMLElement pointer and does not check it. 
			void parseIntegrator(tinyxml2::XMLElement* scene_element);
			void parseCamera(tinyxml2::XMLElement* scene_element);
			void parseOutput(tinyxml2::XMLElement* scene_element);
			void parseObjects(tinyxml2::XMLElement* scene_element);
			void parseLights(tinyxml2::XMLElement* scene_element);
			std::shared_ptr<geometry::Object> parseObject(tinyxml2::XMLElement* object_element);
			void parseMesh(tinyxml2::XMLElement* mesh_element);
			void parseSphere(tinyxml2::XMLElement* sphere_element);
			void parseTriangles(const std::string& datapath);
			std::unique_ptr<core::Filter> parseFilter(tinyxml2::XMLElement* filter_element);
			geometry::Transformation parseTransformation(tinyxml2::XMLElement* transformation_element);
			std::unique_ptr<material::BsdfMaterial> parseBsdfMaterial(tinyxml2::XMLElement* bsdf_material_element);
		};
	}
}

#include "scene.inl"

#endif