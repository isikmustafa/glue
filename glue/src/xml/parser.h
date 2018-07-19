#ifndef __GLUE__XML__PARSER__
#define __GLUE__XML__PARSER__

#include "node.h"
#include "..\core\forward_decl.h"
#include "..\core\scene.h"
#include "..\geometry\bvh.h"

#include <memory>
#include <unordered_set>

namespace glue
{
	namespace xml
	{
		class Parser
		{
		public:
			static const std::unordered_set<std::string> gSupportedFormatsLoad;
			static const std::unordered_set<std::string> gSupportedFormatsSave;

		public:
			static core::Scene::Xml parse(const std::string& xml_filepath);
			static std::shared_ptr<geometry::BVH<geometry::Triangle>> loadModel(const std::string& path);
			static std::shared_ptr<std::vector<core::Image>> loadImage(const std::string& path);
		};
	}
}

#endif