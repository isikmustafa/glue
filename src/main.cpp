#include "core/scene.h"
#include "core/timer.h"
#include "integrator/pathtracer.h"
#include "xml/node.h"

#include <iostream>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "Please indicate input location as the first argument of the program." << std::endl;
        std::cout << "Do not add any other program arguments." << std::endl;
        return 0;
    }

	try
	{
        using namespace glue;
        core::Timer timer;

		timer.start();
        core::Scene scene(core::Scene::Xml(xml::Node::getRoot(argv[1])));
		std::cout << "BVH build and input read time: " << timer.getTime() << std::endl;

		scene.render();
	}
	catch (const std::runtime_error& e)
	{
		std::cout << e.what() << std::endl;
	}

	return 0;
}