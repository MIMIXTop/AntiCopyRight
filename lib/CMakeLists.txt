project(antyCopy_lib)
set(CMAKE_AUTOMOC ON)
file(GLOB_RECURSE SRCS src/*.cpp)
find_package(Boost REQUIRED json)

add_library(antyCopy_lib STATIC ${SRCS})
target_compile_definitions(antyCopy_lib PRIVATE
        MYSTEM_EXECUTABLE="${MYSTEM_EXE}"
)
target_include_directories(antyCopy_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(OpenSSL REQUIRED)
target_link_libraries(antyCopy_lib PUBLIC
        "${TORCH_LIBRARIES}"
        Boost::headers
        Boost::json
        OpenSSL::SSL
        OpenSSL::Crypto
        keychain
        pugixml
        minizip
)

target_include_directories(antyCopy_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

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
        gmock
        antyCopy_lib
)



