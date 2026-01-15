cmake_minimum_required(VERSION 3.20)
project(AntiCopyRight LANGUAGES CXX)

include(FetchContent)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)

set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_WITH_QT6 ON CACHE BOOL "" FORCE)
set(BUILD_WITH_QT5 OFF CACHE BOOL "" FORCE)


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

set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_WITH_QT6 ON  CACHE BOOL "" FORCE)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)

FetchContent_Declare(
        libtorch
        URL ${LIBTORCH_URL}
)

FetchContent_Declare(
        keychain
        GIT_REPOSITORY https://github.com/hrantzsch/keychain.git
        GIT_TAG v1.3.1
)

FetchContent_Declare(
        pugixml
        GIT_REPOSITORY https://github.com/zeux/pugixml.git
        GIT_TAG v1.15
)       

FetchContent_Declare(
        minizip-ng
        GIT_REPOSITORY https://github.com/zlib-ng/minizip-ng.git
        GIT_TAG 4.0.10
)

FetchContent_MakeAvailable(googletest libtorch keychain pugixml minizip-ng)

list(APPEND CMAKE_PREFIX_PATH ${libtorch_SOURCE_DIR})
find_package(Torch REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS
        Core
        Gui
        Quick
        QuickControls2
        QuickLayouts
        QuickTemplates2
        Widgets
)

configure_file(config.hpp.in ${CMAKE_BINARY_DIR}/config.hpp @ONLY)
include_directories(${CMAKE_BINARY_DIR})

add_subdirectory(lib)
target_include_directories(antyCopy_lib PUBLIC ${CMAKE_BINARY_DIR})

file(GLOB_RECURSE SRCS src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.hpp)

qt_add_executable(AntiCopyRight ${SRCS} ${HEADERS})

target_link_libraries(AntiCopyRight PRIVATE Qt6::Core Qt6::Quick Qt6::QuickControls2 Qt6::Widgets)
qt_add_qml_module(AntiCopyRight
        URI QML_SRC
        VERSION 1.0
        QML_FILES
                "src/qml/MainWindow.qml"
        RESOURCES
                "resource/menu-128.png"
                "resource/menu-128.svg"
                "resource/menu-64.svg"
                "resource/DefaultUserLogo.png"
)

add_dependencies(AntiCopyRight MyStemBinary)

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_link_libraries(${PROJECT_NAME} PUBLIC antyCopy_lib Qt6::Quick Qt6::QuickControls2 Qt6::Core)

target_compile_definitions(AntiCopyRight PRIVATE
        MYSTEM_EXECUTABLE="${MYSTEM_EXE}"
)

include(GNUInstallDirs)
install(TARGETS AntiCopyRight
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
