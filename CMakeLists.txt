cmake_minimum_required (VERSION 3.1)
project(pathtracer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

if(MSVC)
	add_compile_options(/MP) # Allow multi-processor building
endif(MSVC)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(deps/embree-windows/embree-config.cmake)

add_subdirectory(shared_code)
add_subdirectory(post1)
