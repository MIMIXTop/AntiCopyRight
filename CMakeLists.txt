cmake_minimum_required(VERSION 3.31)
project(AntiCopyRight LANGUAGES CXX)
include(FetchContent)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    set(MYSTEM_PLATFORM_ARCHIVE "win-64bit.zip")
elseif (APPLE)
    set(MYSTEM_PLATFORM_ARCHIVE "linux-64bit.tar.gz")
else ()
    set(MYSTEM_PLATFORM_ARCHIVE "macosx.tar.gz")
endif ()

set(MYSTEM_VERSION 3.1)
set(MYSTEM_URL     "http://download.cdn.yandex.net/mystem/mystem-${MYSTEM_VERSION}-${MYSTEM_PLATFORM_ARCHIVE}")

ExternalProject_Add(
        MyStemBinary
        URL         ${MYSTEM_URL}
        PREFIX      ${CMAKE_BINARY_DIR}/mystem
        CONFIGURE_COMMAND  ""
        BUILD_COMMAND      ""
        INSTALL_COMMAND    ""
        LOG_DOWNLOAD ON
)

ExternalProject_Get_Property(MyStemBinary SOURCE_DIR)
set(MYSTEM_EXE "${SOURCE_DIR}/mystem" CACHE INTERNAL "Path to MyStem binary")


FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)

FetchContent_Declare(
        quazip
        GIT_REPOSITORY https://github.com/stachenov/quazip.git
        GIT_TAG v1.5
)

FetchContent_MakeAvailable(googletest quazip)

find_package(Qt6 COMPONENTS Core)

add_subdirectory(lib)

file(GLOB_RECURSE SRCS src/*.cpp)

add_executable(AntiCopyRight ${SRCS})
add_dependencies(AntiCopyRight MyStemBinary)

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_link_libraries(${PROJECT_NAME} PUBLIC antyCopy_lib)

target_compile_definitions(AntiCopyRight PRIVATE
        MYSTEM_EXECUTABLE="${MYSTEM_EXE}"
)
