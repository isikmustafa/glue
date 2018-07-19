#ifndef __GLUE__MATERIAL__ORENNAYAR__
#define __GLUE__MATERIAL__ORENNAYAR__

#include "bsdf_material.h"

namespace glue
{
	namespace material
	{
		class OrenNayar : public BsdfMaterial
		{
		public:
			//Xml structure of the class.
			struct Xml : public BsdfMaterial::Xml
			{
				glm::vec3 kd;
				float roughness;

				explicit Xml(const xml::Node& node);
				Xml(const glm::vec3& p_kd, float p_roughness);
				std::unique_ptr<BsdfMaterial> create() const override;
			};

		public:
			OrenNayar(const OrenNayar::Xml& xml);

			std::pair<glm::vec3, glm::vec3> sampleWo(const glm::vec3& wi_tangent, core::UniformSampler& sampler, const geometry::Intersection& intersection) const override;
			glm::vec3 getBsdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const override;
			float getPdf(const glm::vec3& wi_tangent, const glm::vec3& wo_tangent, const geometry::Intersection& intersection) const override;
			bool hasDeltaDistribution() const override;
			bool useMultipleImportanceSampling() const override;

		private:
			glm::vec3 m_kd;
			float m_A;
			float m_B;
		};
	}
}

#endif