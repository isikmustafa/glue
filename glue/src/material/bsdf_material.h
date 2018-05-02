#ifndef __GLUE__MATERIAL__BSDFMATERIAL__
#define __GLUE__MATERIAL__BSDFMATERIAL__

#include "..\core\uniform_sampler.h"

#include <glm\vec3.hpp>
#include <glm\geometric.hpp>

namespace glue
{
	namespace material
	{
		class BsdfMaterial
		{
		public:
			virtual ~BsdfMaterial() {}

			//wi, wo should be in tangent space for sampleWo, getBsdf and getPdf.
			//This makes it easier and efficient to calculate some computations.
			//sampleWo returns <wo, f> pair where f = bsdf * cos / pdf
			virtual std::pair<glm::vec3, glm::vec3> sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler) const = 0;
			virtual glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const = 0;
			virtual float getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent) const = 0;
			virtual bool hasDeltaDistribution() const = 0;
			virtual bool useMultipleImportanceSampling() const = 0;
		};

		//These functions make use of the tangent plane for efficient computations.
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
	}
}

#endif