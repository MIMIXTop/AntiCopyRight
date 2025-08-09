cmake_minimum_required(VERSION 3.31)
project(AntiCopyRight LANGUAGES CXX)

include(FetchContent)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#LibTorch
set(LIBTORCH_VERSION 2.7.1)

if (WIN32)
    #MyStem
    set(MYSTEM_PLATFORM "win")
    set(ARCHIVE "-64bit.zip")

    #LibTorch
    set(LIBTORCH_URL      "https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-${LIBTORCH_VERSION}%2Bcpu.zip")
elseif (APPLE)
    #MyStem
    set(MYSTEM_PLATFORM "macosx")
    set(ARCHIVE ".tar.gz")

    #LibTorch
    set(LIBTORCH_URL     "https://download.pytorch.org/libtorch/cpu/libtorch-macos-arm64-${LIBTORCH_VERSION}.zip")
else ()
    #MyStem
    set(MYSTEM_PLATFORM "linux")
    set(ARCHIVE "-64bit.tar.gz")

    #LibTorch
    set(LIBTORCH_URL     "https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-${LIBTORCH_VERSION}%2Bcpu.zip")
endif ()

set(MYSTEM_VERSION 3.1)
set(MYSTEM_URL     "http://download.cdn.yandex.net/mystem/mystem-${MYSTEM_VERSION}-${MYSTEM_PLATFORM}${ARCHIVE}")

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

FetchContent_Declare(
        libtorch
        URL ${LIBTORCH_URL}
)

FetchContent_MakeAvailable(googletest quazip libtorch)

list(APPEND CMAKE_PREFIX_PATH ${libtorch_SOURCE_DIR})
find_package(Torch REQUIRED)

find_package(Qt6 COMPONENTS
        Core
        Network
        NetworkAuth
        Gui
        Test
)

add_subdirectory(lib)

file(GLOB_RECURSE SRCS src/*.cpp)

add_executable(AntiCopyRight ${SRCS})
add_dependencies(AntiCopyRight MyStemBinary)

add_custom_command(
        TARGET AntiCopyRight POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Utils
        ${CMAKE_BINARY_DIR}/Utils
)

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_link_libraries(${PROJECT_NAME} PUBLIC antyCopy_lib)

target_compile_definitions(AntiCopyRight PRIVATE
        MYSTEM_EXECUTABLE="${MYSTEM_EXE}"
)
