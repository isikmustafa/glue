#include "perlin_texture.h"
#include "..\xml\node.h"
#include "..\core\math.h"

#include <glm\vec3.hpp>
#include <glm\geometric.hpp>

static glm::vec3 gPerlinGradients[] = { glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(-1.0f, -1.0f, 0.0f),
glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, -1.0f),glm::vec3(-1.0f, 0.0f, -1.0f),
glm::vec3(0.0f, 1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 1.0f), glm::vec3(0.0f, 1.0f, -1.0f), glm::vec3(0.0f, -1.0f, -1.0f),
glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, -1.0f), glm::vec3(0.0f, -1.0f, -1.0f) };

static int gPerlinPermutation[] = { 151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 };

namespace glue
{
	namespace texture
	{
		PerlinTexture::Xml::Xml(const xml::Node& node)
		{
			node.parseChildText("First", &first.x, &first.y, &first.z);
			node.parseChildText("Second", &second.x, &second.y, &second.z);
			node.parseChildText("Frequency", &frequency);
		}

		std::unique_ptr<Texture> PerlinTexture::Xml::create() const
		{
			return std::make_unique<PerlinTexture>(*this);
		}

		PerlinTexture::PerlinTexture(const PerlinTexture::Xml& xml)
			: m_first(xml.first)
			, m_second(xml.second)
			, m_frequency(xml.frequency)
		{}

		glm::vec3 PerlinTexture::fetch(const geometry::Intersection& intersection) const
		{
			auto point = intersection.plane.point * m_frequency;
			auto xi = static_cast<int>(std::floor(point.x)) & 255;
			auto yi = static_cast<int>(std::floor(point.y)) & 255;
			auto zi = static_cast<int>(std::floor(point.z)) & 255;

			auto xf = point.x - std::floor(point.x);
			auto yf = point.y - std::floor(point.y);
			auto zf = point.z - std::floor(point.z);

			auto fade = [](float t)
			{
				return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
			};

			//Will be used as weight for linear interpolation.
			auto u = fade(xf);
			auto v = fade(yf);
			auto w = fade(zf);

			//Indexes to select gradient vectors at the edges.
			auto v000 = gPerlinPermutation[gPerlinPermutation[gPerlinPermutation[xi] + yi] + zi];
			auto v010 = gPerlinPermutation[gPerlinPermutation[gPerlinPermutation[xi] + yi + 1] + zi];
			auto v001 = gPerlinPermutation[gPerlinPermutation[gPerlinPermutation[xi] + yi] + zi + 1];
			auto v011 = gPerlinPermutation[gPerlinPermutation[gPerlinPermutation[xi] + yi + 1] + zi + 1];
			auto v100 = gPerlinPermutation[gPerlinPermutation[gPerlinPermutation[xi + 1] + yi] + zi];
			auto v110 = gPerlinPermutation[gPerlinPermutation[gPerlinPermutation[xi + 1] + yi + 1] + zi];
			auto v101 = gPerlinPermutation[gPerlinPermutation[gPerlinPermutation[xi + 1] + yi] + zi + 1];
			auto v111 = gPerlinPermutation[gPerlinPermutation[gPerlinPermutation[xi + 1] + yi + 1] + zi + 1];

			auto grad = [](int gradient_index, float x, float y, float z)
			{
				return glm::dot(gPerlinGradients[gradient_index & 15], glm::vec3(x, y, z));
			};

			auto x1 = core::math::lerp(u, grad(v000, xf, yf, zf), grad(v100, xf - 1.0f, yf, zf));
			auto x2 = core::math::lerp(u, grad(v010, xf, yf - 1.0f, zf), grad(v110, xf - 1.0f, yf - 1.0f, zf));
			auto y1 = core::math::lerp(v, x1, x2);

			x1 = core::math::lerp(u, grad(v001, xf, yf, zf - 1.0f), grad(v101, xf - 1.0f, yf, zf - 1.0f));
			x2 = core::math::lerp(u, grad(v011, xf, yf - 1.0f, zf - 1.0f), grad(v111, xf - 1.0f, yf - 1.0f, zf - 1.0f));
			auto y2 = core::math::lerp(v, x1, x2);

			auto final_weight = (core::math::lerp(w, y1, y2) + 1.0f) * 0.5f;
			return core::math::lerp(final_weight, m_first, m_second);
		}
	}
}