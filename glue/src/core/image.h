#ifndef __GLUE__CORE__IMAGE__
#define __GLUE__CORE__IMAGE__

#include <glm\vec3.hpp>
#include <vector>
#include <string>

namespace glue
{
	namespace core
	{
		class Image
		{
		public:
			Image(int width, int height);
			Image(const std::string& filename);

			std::vector<glm::vec3>& operator[](int i);
			const std::vector<glm::vec3>& operator[](int i) const;
			void saveLdr(const std::string& filename) const;

			int get_width() const { return m_width; }
			int get_height() const { return m_height; }

		private:
			std::vector<std::vector<glm::vec3>> m_pixels;
			int m_width;
			int m_height;
		};
	}
}

#endif