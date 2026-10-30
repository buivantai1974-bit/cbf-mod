cmake_minimum_required(VERSION 3.29)
project(cbf_clone)

set(CMAKE_CXX_STANDARD 20)

add_library(${PROJECT_NAME} SHARED src/main.cpp)

target_link_libraries(${PROJECT_NAME} Geode::Geode)

setup_geode_mod()