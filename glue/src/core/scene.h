#ifndef __GLUE__CORE__SCENE__
#define __GLUE__CORE__SCENE__

#include "pinhole_camera.h"
#include "..\geometry\mesh.h"
#include "..\geometry\bvh.h"
#include "..\geometry\debug_sphere.h"
#include "..\light\light.h"
#include "hdr_image.h"
#include "tonemapper.h"

#include <glm\vec3.hpp>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <tinyxml2.h>

namespace glue
{
	namespace core
	{
		enum class Filter
		{
			BOX,
			GAUSSIAN
		};

		struct Scene
		{
		public:
			//Data
			std::vector<std::shared_ptr<geometry::Mesh>> meshes;
			geometry::BVH bvh;
			std::vector<std::shared_ptr<light::Light>> lights;
			std::unordered_map<const geometry::Mesh*, const light::Light*> light_meshes;
			std::vector<geometry::DebugSphere> debug_spheres;
			geometry::BVH debug_bvh;
			std::vector<std::pair<std::unique_ptr<Tonemapper>, std::string>> output;
			std::unique_ptr<PinholeCamera> camera;
			std::unique_ptr<HdrImage> hdr_image;
			glm::vec3 background_color;
			Filter pixel_filter;
			int sample_count;
			float secondary_ray_epsilon;

		public:
			//Functions
			void loadFromXML(const std::string& filepath);

			template<typename Integrator>
			void render(const Integrator& integrator);

		private:
			std::unordered_map<std::string, std::shared_ptr<std::vector<geometry::Triangle>>> m_path_to_triangles;
			std::unordered_map<std::string, std::shared_ptr<geometry::BVH>> m_path_to_bvh;

		private:
			void parseCamera(tinyxml2::XMLElement* scene_element);
			void parseOutput(tinyxml2::XMLElement* scene_element);
			void parseMeshes(tinyxml2::XMLElement* scene_element);
			void parseLights(tinyxml2::XMLElement* scene_element);
			std::vector<geometry::Triangle> parseTriangles(tinyxml2::XMLElement* datapath_element);
			geometry::Transformation parseTransformation(tinyxml2::XMLElement* transformation_element);
			geometry::Mesh parseMesh(tinyxml2::XMLElement* mesh_element);
			std::unique_ptr<material::BsdfMaterial> parseBsdfMaterial(tinyxml2::XMLElement* bsdf_material_element);
		};
	}
}

#include "scene.inl"

#endif