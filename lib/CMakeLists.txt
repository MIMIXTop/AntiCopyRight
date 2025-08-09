project(antyCopy_lib)
set(CMAKE_AUTOMOC ON)
file(GLOB_RECURSE SRCS src/*.cpp)

add_library(antyCopy_lib STATIC ${SRCS})
target_include_directories(antyCopy_lib PUBLIC src)
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
)

#file(GLOB_RECURSE TEST_SRCS test/*.cpp)

add_executable(antyCopy_tests
        test/test_text_analyzer.cpp
        test/test.cpp
        test/test_model_func.cpp
        test/macros_test.cpp
        test/test_utils.cpp
)
add_dependencies(antyCopy_tests MyStemBinary)
target_compile_definitions(antyCopy_tests PRIVATE
    MYSTEM_EXECUTABLE="${MYSTEM_EXE}"
)
target_link_libraries(antyCopy_tests gtest gtest_main antyCopy_lib)
