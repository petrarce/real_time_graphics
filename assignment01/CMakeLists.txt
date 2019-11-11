cmake_minimum_required(VERSION 3.0)
project(Assignment01)

SET(CMAKE_CXX_STANDARD 17)

# ==============================================================================
# Set bin dir
if(MSVC)
    set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin)
elseif(CMAKE_BUILD_TYPE STREQUAL "")
    set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin/Default)
else()
    set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BIN_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BIN_DIR})

# ===============================================
# Dependencies

# Add TG
add_subdirectory(../libs/typed-geometry ${CMAKE_BINARY_DIR}/libs/typed-geometry)

# Add GLFW lib (with disabled spam)
option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
if(MSVC)
  option(BUILD_SHARED_LIBS "" OFF)
else()
  option(BUILD_SHARED_LIBS "" ON)
endif()
add_subdirectory(../libs/glfw ${CMAKE_BINARY_DIR}/libs/glfw)

# Add GLOW lib
add_subdirectory(../libs/glow ${CMAKE_BINARY_DIR}/libs/glow)
# Add GLOW Extras lib
add_subdirectory(../libs/glow-extras ${CMAKE_BINARY_DIR}/libs/glow-extras)

# Create target
add_executable(Assignment01
    main.cc
    Assignment01.cc
    Assignment01.hh
)

# Link libs
target_link_libraries(Assignment01 PUBLIC 
    typed-geometry
    glow
    glow-extras 
    glfw
)

# Compile flags
if(MSVC)
    target_compile_options(Assignment01 PUBLIC 
        /MP
    )
else()
    target_compile_options(Assignment01 PUBLIC 
        -Wall
    )
endif()
