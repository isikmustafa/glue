#include <png.hpp>
#include <ctpl_stl.h>
#include <thread>

namespace glue
{
	namespace core
	{
		template<typename Integrator>
		void Scene::render(const Integrator& integrator)
		{
			constexpr int cPatchSize = 32;
			auto resolution = camera.get_screen_resolution();
			png::image<png::rgb_pixel> image(resolution.x, resolution.y);
			ctpl::thread_pool pool(std::thread::hardware_concurrency());
			for (int x = 0; x < resolution.x; x += cPatchSize)
			{
				for (int y = 0; y < resolution.y; y += cPatchSize)
				{
					pool.push([x, y, cPatchSize, resolution, this, &integrator, &image](int id)
					{
						for (int i = x; i < x + cPatchSize && i < resolution.x; ++i)
						{
							for (int j = y; j < y + cPatchSize && j < resolution.y; ++j)
							{
								glm::vec3 color = integrator.integratePixel(*this, i, j);
								image[j][i] = png::rgb_pixel(static_cast<png::byte>(color.x), static_cast<png::byte>(color.y), static_cast<png::byte>(color.z));
							}
						}
					});
				}
			}
			pool.stop(true);
			image.write(image_name.c_str());
		}
	}
}