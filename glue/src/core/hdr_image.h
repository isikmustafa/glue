#ifndef __GLUE__CORE__HDRIMAGE__
#define __GLUE__CORE__HDRIMAGE__

#include <glm\vec3.hpp>
#include <vector>
#include <string>

namespace glue
{
	namespace core
	{
		class HdrImage
		{
		public:
			HdrImage(int width, int height);

			std::vector<glm::vec3>& operator[](int x);
			const std::vector<glm::vec3>& operator[](int x) const;
			void save(const std::string& filename) const;
			int width() const;
			int height() const;

		private:
			std::vector<std::vector<glm::vec3>> m_pixels;
		};
	}
}

#endif