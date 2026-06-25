cmake_minimum_required(VERSION 3.10)

# Define the flight software project properties
project(SpaceiumCryoSim 
        VERSION 1.0.0 
        LANGUAGES C)

# Force strict embedded compliance flags (Warnings as errors)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Werror -O2)

#Include directories config
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Production app binary
add_executable(spaceium_cryo_sim 
    main.c 
    cryo_hal.c 
    cryo_controller.c
)

#test bench runner
add_executable(spaceium_cryo_testbench
    test_cryo.c
    cryo_hal.c
    cryo_controller.c
)
