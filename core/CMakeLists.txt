add_library(crystal_lab_core INTERFACE)
add_library(crystal::lab_core ALIAS crystal_lab_core)

target_include_directories(crystal_lab_core
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(crystal_lab_core INTERFACE cxx_std_20)
