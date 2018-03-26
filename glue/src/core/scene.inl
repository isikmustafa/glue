#include <png.hpp>
#include <ctpl_stl.h>

namespace glue
{
	namespace core
	{
		template<typename Integrator>
		void Scene::render(const Integrator& integrator)
		{
			auto resolution = camera.get_screen_resolution();
			png::image<png::rgb_pixel> image(resolution.x, resolution.y);
			ctpl::thread_pool pool(8);
			for (int x = 0; x < resolution.x; ++x)
			{
				pool.push([x, resolution, this, &integrator, &image](int id)
				{
					for (int y = 0; y < resolution.y; ++y)
					{
						glm::vec3 color = integrator.integratePixel(*this, x, y);
						image[y][x] = png::rgb_pixel(static_cast<png::byte>(color.x), static_cast<png::byte>(color.y), static_cast<png::byte>(color.z));
					}
				});
			}
			pool.stop(true);
			image.write(image_name.c_str());
		}
	}
}