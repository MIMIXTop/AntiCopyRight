project(antyCopy_lib)

file(GLOB_RECURSE SRCS src/*.cpp)
add_library(antyCopy_lib STATIC ${SRCS})
target_include_directories(antyCopy_lib PUBLIC src)
target_compile_definitions(antyCopy_lib PRIVATE
        MYSTEM_EXECUTABLE="${MYSTEM_EXE}"
)
target_link_libraries(antyCopy_lib PUBLIC
        QuaZip::QuaZip
        Qt6::Core
        "${TORCH_LIBRARIES}"
)

file(GLOB_RECURSE TEST_SRCS test/*.cpp)

add_executable(antyCopy_tests ${TEST_SRCS})
add_dependencies(antyCopy_tests MyStemBinary)
target_compile_definitions(antyCopy_tests PRIVATE
    MYSTEM_EXECUTABLE="${MYSTEM_EXE}"
)
target_link_libraries(antyCopy_tests gtest gtest_main antyCopy_lib)