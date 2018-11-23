#ifndef __GLUE__MATERIAL__BSDFMATERIAL__
#define __GLUE__MATERIAL__BSDFMATERIAL__

#include "../core/forward_decl.h"

#include <utility>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

namespace glue
{
	namespace material
	{
		class BsdfMaterial
		{
		public:
			//Xml structure of the class.
			struct Xml
			{
				virtual ~Xml() = default;
				virtual std::unique_ptr<BsdfMaterial> create() const = 0;
				static std::unique_ptr<BsdfMaterial::Xml> factory(const xml::Node& node);
			};

		public:
			virtual ~BsdfMaterial() = default;

			//wi, wo should be in tangent space for sampleWo, getBsdf and getPdf.
			//This makes it easier and efficient to calculate some computations.
			//sampleWo returns <wo, f> pair where f = bsdf * cos / pdf
			virtual int chooseBsdf(const glm::vec3& wi_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const { return 0; }
			virtual std::pair<glm::vec3, glm::vec3> sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const = 0;
			virtual glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const = 0;
			virtual float getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const = 0;
			virtual bool hasDeltaDistribution(const geometry::Intersection& intersection) const = 0;
			virtual bool useMultipleImportanceSampling(const geometry::Intersection& intersection) const = 0;
		};
	}
}

#endif