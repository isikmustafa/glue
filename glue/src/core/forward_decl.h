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
		class GaussianFilter;
		class GaussianSampler;
		class HdrImage;
		class PinholeCamera;
		class RealSampler;
		struct Scene;
		class Timer;
		class Tonemapper;
		class Clamp;
		class GlobalReinhard;
		class UniformSampler;
	}

	namespace geometry
	{
		class BBox;
		class BVH;
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
		struct Dielectric;
		struct Conductor;
		class GGXDistribution;
	}
}

#endif