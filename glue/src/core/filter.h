#ifndef __GLUE__CORE__FILTER__
#define __GLUE__CORE__FILTER__

#include "..\core\forward_decl.h"
#include "real_sampler.h"

#include <memory>

namespace glue
{
	namespace core
	{
		class Filter
		{
		public:
			//Xml structure of the class.
			struct Xml
			{
				virtual ~Xml() {}
				virtual std::unique_ptr<Filter> create() const = 0;
				static std::unique_ptr<Filter::Xml> factory(const xml::Node& node);
			};

		public:
			virtual ~Filter() {}

			virtual std::unique_ptr<RealSampler> generateSampler() const = 0;
		};

		class BoxFilter : public Filter
		{
		public:
			//Xml structure of the class.
			struct Xml : public Filter::Xml
			{
				explicit Xml(const xml::Node& node);
				std::unique_ptr<Filter> create() const override;
			};

		public:
			BoxFilter(const BoxFilter::Xml& xml);

			std::unique_ptr<RealSampler> generateSampler() const override;
		};

		class TentFilter : public Filter
		{
		public:
			//Xml structure of the class.
			struct Xml : public Filter::Xml
			{
				explicit Xml(const xml::Node& node);
				std::unique_ptr<Filter> create() const override;
			};

		public:
			TentFilter(const TentFilter::Xml& xml);

			std::unique_ptr<RealSampler> generateSampler() const override;
		};

		class GaussianFilter : public Filter
		{
		public:
			//Xml structure of the class.
			struct Xml : public Filter::Xml
			{
				float sigma;

				explicit Xml(const xml::Node& node);
				std::unique_ptr<Filter> create() const override;
			};

		public:
			GaussianFilter(const GaussianFilter::Xml& xml);

			std::unique_ptr<RealSampler> generateSampler() const override;

		private:
			float m_sigma;
		};
	}
}

#endif