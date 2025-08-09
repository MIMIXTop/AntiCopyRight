# tests/CMakeLists.txt
cmake_minimum_required(VERSION 3.19)
project(NetworkManagerTests LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Test Network NetworkAuth)
# Если Qt5:
# find_package(Qt5 REQUIRED COMPONENTS Core Test Network NetworkAuth)

add_executable(NetworkManagerTest
        NetworkManagerTest.cpp
)

target_include_directories(NetworkManagerTest PRIVATE
        ${CMAKE_SOURCE_DIR}/src    # каталог, где лежат NetworkManager.hpp / ReplyTypes.hpp
)

target_link_libraries(NetworkManagerTest PRIVATE
        Qt6::Core Qt6::Test Qt6::Network Qt6::NetworkAuth
)

# Включить MOC для теста
set_target_properties(NetworkManagerTest PROPERTIES AUTOMOC ON)
