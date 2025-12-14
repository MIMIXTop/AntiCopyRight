project(antyCopy_lib)
set(CMAKE_AUTOMOC ON)
file(GLOB_RECURSE SRCS src/*.cpp)
find_package(Boost REQUIRED json)

add_library(antyCopy_lib STATIC ${SRCS}
        src/Network/keychaintokens.cpp)
target_include_directories(antyCopy_lib PUBLIC src ${QTKEYCHAIN_INCLUDE_DIRS} )
target_compile_definitions(antyCopy_lib PRIVATE
        MYSTEM_EXECUTABLE="${MYSTEM_EXE}"
)
target_link_libraries(antyCopy_lib PUBLIC
        QuaZip::QuaZip
        Qt6::Core
        Qt6::Network
        Qt6::NetworkAuth
        Qt6::Gui
        "${TORCH_LIBRARIES}"
        qt6keychain
        Boost::headers
        Boost::json
)

file(GLOB_RECURSE TEST_SRCS test/*.cpp)

add_executable(antyCopy_tests ${TEST_SRCS})
add_dependencies(antyCopy_tests MyStemBinary)
target_compile_definitions(antyCopy_tests PRIVATE
        MYSTEM_EXECUTABLE="${MYSTEM_EXE}"
        NETWORKMANAGER_TESTS=1
)
target_link_libraries(antyCopy_tests
        gtest
        gtest_main
        Qt6::Test
        antyCopy_lib
)

