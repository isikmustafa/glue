#ifndef __GLUE__CORE__FORWARDDECL__
#define __GLUE__CORE__FORWARDDECL__

namespace glue
{
	namespace core
	{
		class CoordinateSpace;
		class Discrete1DSampler;
		class Discrete2DSampler;
		class Filter;
		class BoxFilter;
		class TentFilter;
		class GaussianFilter;
		class RealSampler;
		class UniformSampler;
		class TentSampler;
		class GaussianSampler;
		class Image;
		class Output;
		class Ldr;
		class PinholeCamera;
		struct Scene;
		class Timer;
		class Tonemapper;
		class Clamp;
		class GlobalReinhard;
	}

	namespace geometry
	{
		class BBox;
		class Sphere;
		struct Intersection;
		class Object;
		class Mesh;
		struct Plane;
		class Ray;
		struct SphericalCoordinate;
		class Transformation;
		class Triangle;
	}

	namespace integrator
	{
		class Integrator;
		class Pathtracer;
	}

	namespace light
	{
		class DiffuseArealight;
		class EnvironmentLight;
		class Light;
	}

	namespace material
	{
		class BsdfMaterial;
		class Dielectric;
		class Lambertian;
		class Metal;
		class OrenNayar;
        class SmoothLayered;
	}

	namespace microfacet
	{
		class BeckmannDistribution;
		class GGXDistribution;

		namespace fresnel
		{
			struct Dielectric;
			struct Conductor;
		}
	}
	
	namespace xml
	{
		class Node;
		class Parser;
	}
}

#endif