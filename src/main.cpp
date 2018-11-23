#include "core/scene.h"
#include "core/timer.h"
#include "integrator/pathtracer.h"
#include "xml/parser.h"

#include <iostream>

int main()
{
	try
	{
		glue::core::Timer timer;

		timer.start();
		//glue::core::Scene scene(glue::xml::Parser::parse("../sample_input/cornell-lucy.xml"));
		glue::core::Scene scene(glue::xml::Parser::parse("../sample_input/material-test.xml"));
		std::cout << "BVH build and input read time: " << timer.getTime() << std::endl;

		scene.render();
	}
	catch (const std::runtime_error& e)
	{
		std::cout << e.what() << std::endl;
		system("PAUSE");
		return 0;
	}

	system("PAUSE");
	return 0;
}