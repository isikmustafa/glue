#ifndef __GLUE__CORE__FORWARDDECL__
#define __GLUE__CORE__FORWARDDECL__

namespace glue
{
	namespace core
	{
		class CoordinateSpace;
		class Discrete1DSampler;
		class Filter;
		class BoxFilter;
		class TentFilter;
		class GaussianFilter;
		class RealSampler;
		class UniformSampler;
		class TentSampler;
		class GaussianSampler;
		class Image;
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
		class DebugSphere;
		struct Intersection;
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
		class Raytracer;
	}

	namespace light
	{
		class DiffuseArealight;
		class Light;
	}

	namespace material
	{
		class BsdfMaterial;
		class Dielectric;
		class Lambertian;
		class Metal;
		class OrenNayar;
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
}

#endif