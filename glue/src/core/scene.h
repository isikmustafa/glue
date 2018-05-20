#ifndef __GLUE__CORE__SCENE__
#define __GLUE__CORE__SCENE__

#include "pinhole_camera.h"
#include "..\geometry\mesh.h"
#include "..\geometry\bvh.h"
#include "..\geometry\debug_sphere.h"
#include "..\light\light.h"
#include "image.h"
#include "tonemapper.h"

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
			std::vector<std::shared_ptr<geometry::Mesh>> meshes;
			geometry::BVH bvh;
			std::vector<geometry::DebugSphere> debug_spheres;
			geometry::BVH debug_bvh;
			std::vector<std::shared_ptr<light::Light>> lights;
			std::unordered_map<const geometry::Mesh*, const light::Light*> light_meshes;
			std::vector<std::pair<std::unique_ptr<Tonemapper>, std::string>> output;
			std::unique_ptr<PinholeCamera> camera;
			std::unique_ptr<Image> image;
			glm::vec3 background_radiance;
			float secondary_ray_epsilon;

		public:
			void loadFromXML(const std::string& filepath);

			void render();

		private:
			std::unique_ptr<integrator::Integrator> m_integrator;
			std::unordered_map<std::string, std::shared_ptr<std::vector<geometry::Triangle>>> m_path_to_triangles;
			std::unordered_map<std::string, std::shared_ptr<geometry::BVH>> m_path_to_bvh;
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
			void parseMeshes(tinyxml2::XMLElement* scene_element);
			void parseLights(tinyxml2::XMLElement* scene_element);
			void parseMesh(tinyxml2::XMLElement* mesh_element);
			void parseTriangles(const std::string& datapath);
			std::unique_ptr<core::Filter> parseFilter(tinyxml2::XMLElement* filter_element);
			geometry::Transformation parseTransformation(tinyxml2::XMLElement* transformation_element);
			std::unique_ptr<material::BsdfMaterial> parseBsdfMaterial(tinyxml2::XMLElement* bsdf_material_element);
		};
	}
}

#include "scene.inl"

#endif