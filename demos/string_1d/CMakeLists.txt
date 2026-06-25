add_executable(string_1d
    src/main.cpp
    src/String1DSimulation.cpp
)

target_include_directories(string_1d
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(string_1d PRIVATE crystal::lab_core)

target_compile_features(string_1d PRIVATE cxx_std_20)

if (BUILD_TESTING)
    add_test(
        NAME string_1d_smoke
        COMMAND string_1d
            --steps 40
            --observe-every 10
            --output string_1d_smoke.csv
            --profile-output string_1d_smoke_profile.csv
    )
endif()
