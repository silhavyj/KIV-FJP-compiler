CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

project(FJPCompiler)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    string(REGEX REPLACE "/W[1-3]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/include/")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/lib/")

FILE(GLOB_RECURSE src_files "src/*.cpp")

ADD_EXECUTABLE(fjp ${src_files})