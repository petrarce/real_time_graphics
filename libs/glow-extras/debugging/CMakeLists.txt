cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE SHADER_FILES "shader/*.*")

if (GLOW_EXTRAS_EMBED_SHADERS)
    embed_files("shader/" "debugging_embed_shaders" "${SHADER_FILES}")
endif()

file(GLOB_RECURSE SOURCE_FILES "*.cc")
file(GLOB_RECURSE HEADER_FILES "*.hh")

target_sources(glow-extras PRIVATE ${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})
target_include_directories(glow-extras PUBLIC "./")
target_compile_definitions(glow-extras PUBLIC GLOW_EXTRAS_DEBUGGING)
