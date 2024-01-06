cmake_minimum_required(VERSION 3.17)
project(wzlib)

set(CMAKE_CXX_STANDARD 20)

set(ZLIB_LIBRARY "zlib/lib")
set(ZLIB_INCLUDE_DIR "zlib/include")

MESSAGE(${CMAKE_CXX_COMPILER_ID})
       
find_package(ZLIB REQUIRED)

include_directories(
        .
        include
        include/wz
        mio/include
        pystring
        ${ZLIB_INCLUDE_DIR})

file(GLOB SOURCE_FILES src/*.cpp pystring/pystring.cpp)
file(GLOB AES_SOURCE_FILES AES/AES.cpp)

add_library(wzlib ${SOURCE_FILES} ${AES_SOURCE_FILES})
target_link_libraries(ZLIB::ZLIB)


project(wzlibtest)
file(GLOB LIB_FILES *.a ${ZLIB_LIBRARY}/*.a)

add_executable(wzlibtest main/main.cpp ${SOURCE_FILES})
target_link_libraries(
        wzlibtest
        PRIVATE
        wzlib
        ${LIB_FILES}  
)