set(SOURCES shared.h shared.cpp vector_math.h)
file(GLOB_RECURSE EMBREE_SOURCES LIST_DIRECTORIES false "../deps/embree-windows/include/*.h")

add_library(shared_code ${SOURCES} ${EMBREE_SOURCES})

source_group("source" FILES ${SOURCES})
source_group("embree" FILES ${EMBREE_SOURCES})

target_include_directories(shared_code PRIVATE "../deps/stb")
target_include_directories(shared_code PRIVATE ${EMBREE_INCLUDE_DIRS})
target_include_directories(shared_code PUBLIC ".")

target_link_libraries(shared_code PRIVATE ${EMBREE_LIBRARIES})
target_compile_definitions(shared_code PRIVATE _CRT_SECURE_NO_WARNINGS)

# TODO: We must move embree.dll to binary directory