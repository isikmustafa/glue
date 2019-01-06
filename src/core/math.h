#ifndef __GLUE__CORE__MATH__
#define __GLUE__CORE__MATH__

#include "../geometry/spherical_coordinate.h"

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>

namespace glue
{
	namespace core
	{
		namespace math
		{
			//From "Approximating the erfinv function" by Mike Giles.
			inline float inverseErf(float x)
			{
				float p;
				auto w = -glm::log((1.0f - x) * (1.0f + x));
				if (w < 5.0f)
				{
					w = w - 2.5f;
					p = 2.81022636e-08f;
					p = glm::fma(p, w, 3.43273939e-07f);
					p = glm::fma(p, w, -3.5233877e-06f);
					p = glm::fma(p, w, -4.39150654e-06f);
					p = glm::fma(p, w, 0.00021858087f);
					p = glm::fma(p, w, -0.00125372503f);
					p = glm::fma(p, w, -0.00417768164f);
					p = glm::fma(p, w, 0.246640727f);
					p = glm::fma(p, w, 1.50140941f);
				}
				else
				{
					w = glm::sqrt(w) - 3.0f;
					p = -0.000200214257f;
					p = glm::fma(p, w, 0.000100950558f);
					p = glm::fma(p, w, 0.00134934322f);
					p = glm::fma(p, w, -0.00367342844f);
					p = glm::fma(p, w, 0.00573950773f);
					p = glm::fma(p, w, -0.0076224613f);
					p = glm::fma(p, w, 0.00943887047f);
					p = glm::fma(p, w, 1.00167406f);
					p = glm::fma(p, w, 2.83297682f);
				}
				return p * x;
			}

			inline unsigned int closestPowTwo(unsigned int number)
			{
				auto v = number;
				--v;
				v |= v >> 1;
				v |= v >> 2;
				v |= v >> 4;
				v |= v >> 8;
				v |= v >> 16;
				++v;

				return (v - number) > (v >> 2) ? (v >> 1) : v;
			}

			inline float rgbToLuminance(const glm::vec3& rgb)
			{
				return glm::dot(glm::vec3(0.2126f, 0.7152f, 0.0722f), rgb);
			}

			template<typename Value>
			inline Value lerp(float t, Value v0, Value v1)
			{
				return v0 + t * (v1 - v0);
			}

			//Following trigonometric functions assumes the parameters are given in tangent space.
			//This results in efficient computations.
			inline float cosTheta(const glm::vec3& w_tangent)
			{
				return w_tangent.z;
			}

			inline float cos2Theta(const glm::vec3& w_tangent)
			{
				return w_tangent.z * w_tangent.z;
			}

			inline float sinTheta(const glm::vec3& w_tangent)
			{
				return glm::sqrt(1.0f - w_tangent.z * w_tangent.z);
			}

			inline float sin2Theta(const glm::vec3& w_tangent)
			{
				return 1.0f - w_tangent.z * w_tangent.z;
			}

			inline float tanTheta(const glm::vec3& w_tangent)
			{
				return glm::sqrt((1.0f - w_tangent.z * w_tangent.z) / (w_tangent.z * w_tangent.z));
			}

			inline float tan2Theta(const glm::vec3& w_tangent)
			{
				return (1.0f - w_tangent.z * w_tangent.z) / (w_tangent.z * w_tangent.z);
			}

			inline geometry::SphericalCoordinate sampleHemisphereCosine(float u1, float u2)
			{
				return geometry::SphericalCoordinate(1.0f, glm::acos(glm::sqrt(u1)), glm::two_pi<float>() * u2);
			}

			inline geometry::SphericalCoordinate sampleHemisphereUniform(float u1, float u2)
			{
				return geometry::SphericalCoordinate(1.0f, glm::acos(u1), glm::two_pi<float>() * u2);
			}

			inline geometry::SphericalCoordinate sampleSphereUniform(float u1, float u2)
			{
				return geometry::SphericalCoordinate(1.0f, glm::acos(1.0f - 2.0f * u1), glm::two_pi<float>() * u2);
			}
		}
	}
}

#endif