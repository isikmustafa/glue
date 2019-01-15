cmake_minimum_required(VERSION 3.9)
project(glue)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(3rd/tinyxml2)

include_directories("3rd/glm")
include_directories("3rd/stb")
include_directories("3rd/tinyxml2")
include_directories("3rd/tinyobjloader")

link_directories("${CMAKE_CURRENT_BINARY_DIR}/3rd/tinyxml2/")

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(SOURCE_FILES
        src/main.cpp

        src/core/timer.cpp src/core/coordinate_space.cpp src/core/discrete_1d_sampler.cpp
        src/core/discrete_2d_sampler.cpp src/core/filter.cpp src/core/image.cpp src/core/image.cpp src/core/output.cpp
        src/core/pinhole_camera.cpp src/core/real_sampler.cpp src/core/scene.cpp src/core/timer.cpp src/core/timer.cpp
        src/core/tonemapper.cpp

        src/geometry/bbox.cpp src/geometry/mapper.cpp src/geometry/mesh.cpp src/geometry/object.cpp src/geometry/plane.cpp
        src/geometry/ray.cpp src/geometry/sphere.cpp src/geometry/spherical_coordinate.cpp src/geometry/transformation.cpp
        src/geometry/triangle.cpp

        src/integrator/integrator.cpp src/integrator/pathtracer.cpp src/integrator/sppm.cpp

        src/light/diffuse_arealight.cpp src/light/environment_light.cpp src/light/light.cpp src/light/pointlight.cpp

        src/material/bsdf_material.cpp src/material/dielectric.cpp src/material/lambertian.cpp src/material/metal.cpp
        src/material/oren_nayar.cpp src/material/smooth_layered.cpp

        src/microfacet/beckmann_distribution.cpp src/microfacet/fresnel.cpp src/microfacet/ggx_distribution.cpp

        src/texture/checkerboard_2d_texture.cpp src/texture/checkerboard_3d_texture.cpp src/texture/constant_texture.cpp
        src/texture/image_texture.cpp src/texture/perlin_texture.cpp src/texture/texture.cpp

        src/xml/node.cpp src/xml/parser.cpp
        )

add_executable(glue ${SOURCE_FILES})

#Change "libtinyxml2d.dll.a" and "libtinyxml2.dll.a" based on your platform and compiler.
target_link_libraries(glue debug "libtinyxml2d.dll.a" optimized "libtinyxml2.dll.a")