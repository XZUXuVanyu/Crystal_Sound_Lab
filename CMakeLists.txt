# Global Language Configuration
cmake_minimum_required(VERSION 3.28)
project(Crystal_Sound_Lab LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Name Aliases
set(CORE_LIB Crystal_Sound_Lab_Core)
set(CORE_LIB_EXT_DEPENDENCIES "")
set(CORE_MODULES
    src/core/Foundation.ixx
    src/core/math/Complex.ixx
    src/core/nodes/AudioNodes.ixx
)

# Crystal_Sound_Lab_Core Configuration
add_library(${CORE_LIB} STATIC)
target_sources(${CORE_LIB}
    PUBLIC
        FILE_SET CXX_MODULES FILES
            ${CORE_MODULES})

target_include_directories()
add_executable(Crystal_Sound_Lab src/Source.cpp)
target_link_libraries(Crystal_Sound_Lab 
    PRIVATE 
        ${CORE_LIB}
        Crystal_Log)