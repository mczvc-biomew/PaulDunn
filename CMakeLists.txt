cmake_minimum_required(VERSION 3.16.3)

project(PaulDunn)
project(ChaosGame)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)
file(GLOB PD_SRCS
     "${PROJECT_SOURCE_DIR}/include/egg2d.h"
     "${PROJECT_SOURCE_DIR}/include/fractal_renderer.hpp"
     "${PROJECT_SOURCE_DIR}/include/pbcolor.hpp"
        "${PROJECT_SOURCE_DIR}/src/egg2d.c"
        "${PROJECT_SOURCE_DIR}/src/glad.c"
        "${PROJECT_SOURCE_DIR}/src/fractal_renderer.cpp"
        "${PROJECT_SOURCE_DIR}/src/main.cpp"
     )

file(GLOB CG_SRCS
    "${PROJECT_SOURCE_DIR}/include/egg2d.h"
    "${PROJECT_SOURCE_DIR}/include/chaosgame.hpp"
        "${PROJECT_SOURCE_DIR}/src/egg2d.c"
        "${PROJECT_SOURCE_DIR}/src/glad.c"
        "${PROJECT_SOURCE_DIR}/src/chaosgame.cpp"
        "${PROJECT_SOURCE_DIR}/src/chaos_main.cpp"
)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++11 -ldl")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_definitions(-DGLES2)
endif ()

add_executable(PaulDunn ${PD_SRCS})
add_executable(ChaosGame ${CG_SRCS})

find_package(OpenGL REQUIRED COMPONENTS OpenGL)
find_package(SDL2 REQUIRED)

include_directories(${SDL_IMAGE_INCLUDE_DIR} ${OPENGL_INCLUDE_DIRS})

target_link_libraries(PaulDunn SDL2::SDL2 ${OPENGL_LIBRARIES})
target_link_libraries(ChaosGame SDL2::SDL2 ${OPENGL_LIBRARIES})
