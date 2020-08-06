cmake_minimum_required(VERSION 3.17)
project(wzlibcpp)

set(CMAKE_CXX_STANDARD 17)

include(F:/vcpkg/scripts/buildsystems/vcpkg.cmake)

find_package(ZLIB REQUIRED)

include_directories(
        include
        ${ZLIB_INCLUDE_DIR})

file(GLOB SOURCE_FILES src/*.cpp)

add_library(wzlibcpp ${SOURCE_FILES})
target_link_libraries(ZLIB::ZLIB)
