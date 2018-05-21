#include "core\scene.h"
#include "core\timer.h"
#include "integrator\pathtracer.h"

#include <iostream>

int main()
{
	glue::core::Scene scene;
	glue::core::Timer timer;
	timer.start();
	try
	{
		scene.loadFromXML("../sample_input/cornell-lucy.xml");
	}
	catch (const std::runtime_error& e)
	{
		std::cout << e.what() << std::endl;
		system("PAUSE");
		return 0;
	}
	std::cout << "BVH build and input read time: " << timer.getTime() << std::endl;

	timer.start();
	scene.render();
	std::cout << "Render time: " << timer.getTime() << std::endl;

	system("PAUSE");
	return 0;
}