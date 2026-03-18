# Global Language Configuration
cmake_minimum_required(VERSION 3.28)
project(Crystal_Sound_Lab LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Name Aliases
set(APP_NAME Crystal_Sound_Lab)
set(CORE_LIB Crystal_Sound_Lab_Core)
set(CORE_LIB_EXT_DEPENDENCIES "")
set(CORE_MODULES
    src/core/Foundation.ixx
    src/core/math/Complex.ixx
    src/core/nodes/AudioNodes.ixx
)
set(JUCE_LIB
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_opengl
)
# External Dependencies
add_subdirectory("external/Crystal_Log")
add_subdirectory("external/JUCE")
# Crystal_Sound_Lab_Core Configuration
add_library(${CORE_LIB} STATIC)
target_sources(${CORE_LIB}
    PUBLIC
        FILE_SET CXX_MODULES FILES
            ${CORE_MODULES}
)
target_include_directories(${CORE_LIB}
    PUBLIC
        "external/includes"
)
target_link_libraries(${CORE_LIB}
    PUBLIC
        ${JUCE_LIB}
)

# Crystal_Sound_Lab Configuration
add_executable(${APP_NAME} src/Source.cpp)

target_link_libraries(${APP_NAME}
    PRIVATE 
        ${CORE_LIB}
        Crystal_Log
)