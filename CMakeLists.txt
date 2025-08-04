cmake_minimum_required(VERSION 3.31)
project(AntiCopyRight)
include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
        quazip
        GIT_REPOSITORY https://github.com/stachenov/quazip.git
        GIT_TAG v1.5
)
FetchContent_MakeAvailable(quazip)

FetchContent_Declare(
        libtorch
        GIT_REPOSITORY
)

find_package(Qt6 COMPONENTS Core)

add_subdirectory(lib)

file(GLOB_RECURSE SRCS src/*.cpp)
add_executable(AntiCopyRight ${SRCS})
target_include_directories(${PROJECT_NAME} PUBLIC src)
target_link_libraries(${PROJECT_NAME} PUBLIC antyCopy_lib)
