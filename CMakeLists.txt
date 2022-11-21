cmake_minimum_required(VERSION 3.16.3)

project(PaulDunn)
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)
file(GLOB SRCS
     "${PROJECT_SOURCE_DIR}/include/*"
     "${PROJECT_SOURCE_DIR}/src/*"
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

add_executable(${PROJECT_NAME} ${SRCS})

find_package(OpenGL REQUIRED COMPONENTS OpenGL)
find_package(SDL2 REQUIRED)

include_directories(${SDL_IMAGE_INCLUDE_DIR} ${OPENGL_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} SDL2::SDL2 ${OPENGL_LIBRARIES})
