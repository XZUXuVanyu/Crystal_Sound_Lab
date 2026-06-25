cmake_minimum_required(VERSION 3.25)

project(CrystalSoundLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CRYSTAL_LAB_BUILD_DEMOS "Build the physics demos" ON)

include(CTest)

add_subdirectory(core)

if (CRYSTAL_LAB_BUILD_DEMOS)
    add_subdirectory(demos/string_1d)
endif()
