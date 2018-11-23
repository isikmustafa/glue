#ifndef __GLUE__CORE__TONEMAPPER__
#define __GLUE__CORE__TONEMAPPER__

#include "image.h"
#include "../core/forward_decl.h"

#include <memory>

namespace glue
{
	namespace core
	{
		class Tonemapper
		{
		public:
			//Xml structure of the class.
			struct Xml
			{
				virtual ~Xml() = default;
				virtual std::unique_ptr<Tonemapper> create() const = 0;
				static std::unique_ptr<Tonemapper::Xml> factory(const xml::Node& node);
			};

		public:
			virtual ~Tonemapper() = default;

			virtual Image tonemap(const Image& image) const = 0;
		};

		//Clamps the pixel values that are out of the range to min and max.
		class Clamp : public Tonemapper
		{
		public:
			//Xml structure of the class.
			struct Xml : public Tonemapper::Xml
			{
				float min;
				float max;

				explicit Xml(const xml::Node& node);
				std::unique_ptr<Tonemapper> create() const override;
			};

		public:
			explicit Clamp(const Clamp::Xml& xml);

			Image tonemap(const Image& image) const override;

		private:
			float m_min;
			float m_max;
		};

		//From Reinhard et al., 2002.
		class GlobalReinhard : public Tonemapper
		{
		public:
			//Xml structure of the class.
			struct Xml : public Tonemapper::Xml
			{
				float key;
				float max_luminance;

				explicit Xml(const xml::Node& node);
				std::unique_ptr<Tonemapper> create() const override;
			};

		public:
			explicit GlobalReinhard(const GlobalReinhard::Xml& xml);

			Image tonemap(const Image& image) const override;

		private:
			//(maximum luminance of the scene) * max_luminance will be represented as 1.0.
			//e.g. 0.5 means half of (maximum luminance of the scene) will be mapped to 1.0 and the rest will be burnt out.
			float m_key;
			float m_max_luminance;
		};
	}
}

#endif