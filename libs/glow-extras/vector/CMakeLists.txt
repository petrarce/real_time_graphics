cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE SOURCE_FILES "*.cc")
file(GLOB_RECURSE HEADER_FILES "*.hh")

target_sources(glow-extras PRIVATE ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(glow-extras PUBLIC "./")
target_compile_definitions(glow-extras PUBLIC GLOW_EXTRAS_TIMING)

if (GLOW_EXTRAS_DEFAULT_FONTS)
    target_compile_definitions(glow-extras PUBLIC GLOW_EXTRAS_DEFAULT_FONTS)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/fonts/FiraMono-Regular.ttf" "${GLOW_BIN_DIR}/FiraMono-Regular.ttf" COPYONLY)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/fonts/FiraSans-Regular.ttf" "${GLOW_BIN_DIR}/FiraSans-Regular.ttf" COPYONLY)
endif()
