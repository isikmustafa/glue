#ifndef __GLUE__CORE__PARSER__
#define __GLUE__CORE__PARSER__

#include "camera.h"
#include "..\geometry\triangle.h"
#include "..\geometry\transformation.h"
#include "..\geometry\mesh.h"

#include <tinyxml2.h>
#include <vector>
#include <unordered_map>

namespace glue
{
	namespace core
	{
		namespace parser
		{
			Camera parseCamera(tinyxml2::XMLElement* camera_element);
			std::vector<geometry::Triangle> parseTriangles(tinyxml2::XMLElement* datapath_element);
			geometry::Transformation parseTransformation(tinyxml2::XMLElement* transformation_element);
			geometry::Mesh parseMesh(tinyxml2::XMLElement* mesh_element
				, std::unordered_map<std::string, std::shared_ptr<std::vector<geometry::Triangle>>>& path_to_triangles
				, std::unordered_map<std::string, std::shared_ptr<geometry::BVH>>& path_to_bvh);
			std::unique_ptr<material::BsdfMaterial> parseBsdfMaterial(tinyxml2::XMLElement* bsdf_material_element);
		}
	}
}

#endif