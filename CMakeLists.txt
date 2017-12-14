cmake_minimum_required (VERSION 3.1)
project(pathtracer)

macro(set_linker_options name)
	if(MSVC)
		target_link_libraries(${name} PRIVATE -LTCG) # Link time code generation
		target_link_libraries(${name} PRIVATE -INCREMENTAL:NO) # Link time code generation
	endif(MSVC)
endmacro(set_linker_options)

macro(add_post name)
	set(SOURCES ${name}.cpp)
	add_executable(${name} ${SOURCES})
	source_group("source" FILES ${SOURCES})
	target_link_libraries(${name} PRIVATE shared_code)
	set_linker_options(${name})
	install(TARGETS ${name} DESTINATION ".")
endmacro(add_post)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

if(MSVC)
	add_compile_options(/MP) # Allow multi-processor building
	add_compile_options(/GL) # Whole Program Optimization
endif(MSVC)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(deps/embree-windows/embree-config.cmake)

add_subdirectory(shared_code)
add_subdirectory(post1)
add_subdirectory(post2)
add_subdirectory(post3)
add_subdirectory(post4)
