#include "filter.h"
#include "real_sampler.h"
#include "..\xml\node.h"

namespace glue
{
	namespace core
	{
		std::unique_ptr<Filter::Xml> Filter::Xml::factory(const xml::Node& node)
		{
			auto filter_type = node.attribute("type", true);

			if (filter_type == std::string("Box"))
			{
				return std::make_unique<BoxFilter::Xml>(node);
			}
			else if (filter_type == std::string("Tent"))
			{
				return std::make_unique<TentFilter::Xml>(node);
			}
			else if (filter_type == std::string("Gaussian"))
			{
				return std::make_unique<GaussianFilter::Xml>(node);
			}
			else
			{
				node.throwError("Unknown Filter type.");
			}
		}

		BoxFilter::Xml::Xml(const xml::Node& node)
		{}

		std::unique_ptr<Filter> BoxFilter::Xml::create() const
		{
			return std::make_unique<BoxFilter>(*this);
		}

		BoxFilter::BoxFilter(const BoxFilter::Xml& xml)
		{}

		std::unique_ptr<RealSampler> BoxFilter::generateSampler() const
		{
			return std::make_unique<UniformSampler>(-0.5f, 1.5f);
		}

		//Tent
		TentFilter::Xml::Xml(const xml::Node& node)
		{}

		std::unique_ptr<Filter> TentFilter::Xml::create() const
		{
			return std::make_unique<TentFilter>(*this);
		}

		TentFilter::TentFilter(const TentFilter::Xml& xml)
		{}

		std::unique_ptr<RealSampler> TentFilter::generateSampler() const
		{
			return std::make_unique<TentSampler>(0.5f, 1.0f);
		}

		//Gaussian
		GaussianFilter::Xml::Xml(const xml::Node& node)
		{
			node.parseChildText("Sigma", &sigma);
		}

		std::unique_ptr<Filter> GaussianFilter::Xml::create() const
		{
			return std::make_unique<GaussianFilter>(*this);
		}

		GaussianFilter::GaussianFilter(const GaussianFilter::Xml& xml)
			: m_sigma(xml.sigma)
		{}

		std::unique_ptr<RealSampler> GaussianFilter::generateSampler() const
		{
			return std::make_unique<GaussianSampler>(0.5f, m_sigma);
		}
	}
}