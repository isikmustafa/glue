#ifndef __GLUE__CORE__PARSER__
#define __GLUE__CORE__PARSER__

#include "camera.h"
#include "..\geometry\triangle.h"

#include <tinyxml2.h>
#include <vector>

namespace glue
{
	namespace core
	{
		namespace parser
		{
			Camera parseCamera(tinyxml2::XMLElement* camera_element);
			std::vector<geometry::Triangle> parseTriangles(tinyxml2::XMLElement* mesh_element);
		}
	}
}

#endif