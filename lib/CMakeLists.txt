project(antyCopy_lib)

file(GLOB_RECURSE SRCS src/*.cpp)
add_library(antyCopy_lib STATIC ${SRCS})
target_include_directories(antyCopy_lib PUBLIC src)
target_link_libraries(antyCopy_lib PUBLIC
        QuaZip::QuaZip
        Qt6::Core
)

file(GLOB_RECURSE TEST_SRCS test/*.cpp)

add_executable(antyCopy_tests ${TEST_SRCS})
target_link_libraries(antyCopy_tests gtest gtest_main antyCopy_lib)