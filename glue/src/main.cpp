#include "core\scene.h"
#include "core\timer.h"
#include "integrator\raytracer.h"
#include "integrator\pathtracer.h"

#include <iostream>

int main()
{
	glue::core::Scene scene;
	glue::core::Timer timer;
	timer.start();
	scene.loadFromXML("../sample_input/cornell-lucy.xml");
	std::cout << "BVH build and input read time: " << timer.getTime() << std::endl;

	timer.start();
	scene.render();
	std::cout << "Render time: " << timer.getTime() << std::endl;

	system("PAUSE");
	return 0;
}