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
			auto resolution = camera->get_screen_resolution();
			ctpl::thread_pool pool(std::thread::hardware_concurrency());
			for (int x = 0; x < resolution.x; x += cPatchSize)
			{
				for (int y = 0; y < resolution.y; y += cPatchSize)
				{
					pool.push([x, y, cPatchSize, resolution, this, &integrator](int id)
					{
						for (int i = x; i < x + cPatchSize && i < resolution.x; ++i)
						{
							for (int j = y; j < y + cPatchSize && j < resolution.y; ++j)
							{
								(*this->hdr_image)[i][j] = integrator.integratePixel(*this, i, j);
							}
						}
					});
				}
			}
			pool.stop(true);

			for (const auto& image : output)
			{
				image.first->tonemap(*hdr_image).save(image.second);
			}
		}
	}
}