cmake_minimum_required(VERSION 3.17)
project(wzlibcpp)

set(CMAKE_CXX_STANDARD 17)

set(ZLIB_LIBRARY "zlib/lib")
set(ZLIB_INCLUDE_DIR "zlib/include")

set(CMAKE_CXX_FLAGS "-fpermissive")
MESSAGE(${CMAKE_CXX_COMPILER_ID})
       
find_package(ZLIB REQUIRED)

include_directories(
        .
        include
        include/wz
        mio/include
        ${ZLIB_INCLUDE_DIR})

file(GLOB SOURCE_FILES src/*.cpp)
file(GLOB AES_SOURCE_FILES AES/AES.cpp)



add_library(wzlibcpp ${SOURCE_FILES} ${AES_SOURCE_FILES})
target_link_libraries(ZLIB::ZLIB)
