cmake_minimum_required(VERSION 3.17)
project(wzlibcpp)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(zlib)

set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib)

include_directories(
        .
        include
        include/wz
        mio/include
        ${ZLIB_INCLUDE_DIR})

file(GLOB SOURCE_FILES src/*.cpp)
file(GLOB AES_SOURCE_FILES AES/AES.cpp)

add_library(wzlib ${SOURCE_FILES} ${AES_SOURCE_FILES})
target_link_libraries(wzlib zlibstatic)


project(wzlibtest)
file(GLOB LIB_FILES *.a ${ZLIB_LIBRARY}/*.a)

add_executable(wzlibtest main/main.cpp ${SOURCE_FILES})
target_link_libraries(
        wzlibtest
        PRIVATE
        wzlib
        ${LIB_FILES}  
)