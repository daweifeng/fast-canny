cmake_minimum_required(VERSION 3.22)

project(FastCanny)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


option(BUILD_BENCHMARK "Build benchmark" ON)


if (BUILD_BENCHMARK)
  add_subdirectory(benchmark)
endif()
