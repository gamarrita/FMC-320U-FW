cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME fmc-320u-v2)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)


# Select active app
set(FM_AVAILABLE_APPS
    product/main
    template
    bringups/debug_panic
    bringups/lcd
    bringups/lcd_blink
    bringups/display_format_lcd
    bringups/keyboard_input
    bringups/pulse_counter
    bringups/frequency_observation
    tests/regression
)

# Edit this value to select the firmware app built by the canonical flow.
set(FM_ACTIVE_APP_DEFAULT "product/main")

# Remove selector entries stored by older revisions.
unset(FM_ACTIVE_APP CACHE)
unset(FM_ACTIVE_APP_LAST_DEFAULT CACHE)
set(FM_ACTIVE_APP "${FM_ACTIVE_APP_DEFAULT}")

set(FM_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

if(NOT FM_ACTIVE_APP IN_LIST FM_AVAILABLE_APPS)
    message(FATAL_ERROR
        "Invalid FM_ACTIVE_APP='${FM_ACTIVE_APP}'. "
        "Available apps: ${FM_AVAILABLE_APPS}"
    )
endif()

set(FM_APP_DIR "${FM_SRC_DIR}/apps/${FM_ACTIVE_APP}")
message(STATUS "Active app: ${FM_ACTIVE_APP}")

# Execute child CMakeLists.txt of the active project
add_subdirectory(${FM_APP_DIR})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# CubeMX records ThreadX low-power as an assembler define in .mxproject, but
# the generated CMake target does not propagate it. The canonical CMake build
# needs this define so tx_thread_schedule.S calls the official low-power utility.
if(TARGET ThreadX)
    target_compile_definitions(ThreadX PRIVATE TX_LOW_POWER)
endif()

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Shared sources for every app.
set(FM_COMMON_SOURCES
    ${FM_SRC_DIR}/apps/fm_app_threadx.c
    ${FM_SRC_DIR}/libs/fm_debug.c
    ${FM_SRC_DIR}/bsp/fm_board.c
    ${FM_SRC_DIR}/bsp/fm_board_keyboard.c
    ${FM_SRC_DIR}/bsp/devices/lcd/fm_lcd.c
    ${FM_SRC_DIR}/bsp/devices/lcd/fm_lcd_blink.c
    ${FM_SRC_DIR}/bsp/devices/lcd/fm_lcd_map.c
    ${FM_SRC_DIR}/bsp/devices/lcd/pcf8553/fm_pcf8553.c
    ${FM_SRC_DIR}/product/fmc/fmc_model.c
    ${FM_SRC_DIR}/product/fmc/fmc_presentation.c
    ${FM_SRC_DIR}/product/fmc/fmc_rate.c
    ${FM_SRC_DIR}/product/fmc/fmc_runtime.c
    ${FM_SRC_DIR}/product/fmc/fmc_service.c
    ${FM_SRC_DIR}/product/fmc/fmc_units.c
    ${FM_SRC_DIR}/product/fmc/fmc_volume.c
    ${FM_SRC_DIR}/services/display_format.c
    ${FM_SRC_DIR}/services/frequency_observation.c
    ${FM_SRC_DIR}/services/pulse_delta.c
    ${FM_SRC_DIR}/port/fm_port_dwt.c
    ${FM_SRC_DIR}/port/fm_port_frequency_time.c
    ${FM_SRC_DIR}/port/fm_port_gpio.c
    ${FM_SRC_DIR}/port/fm_port_gpio_exti.c
    ${FM_SRC_DIR}/port/fm_port_pcf8553_ctrl.c
    ${FM_SRC_DIR}/port/fm_port_pulse_counter.c
    ${FM_SRC_DIR}/port/fm_port_rcc.c
    ${FM_SRC_DIR}/port/fm_port_rtc.c
    ${FM_SRC_DIR}/port/fm_port_spi1.c
    ${FM_SRC_DIR}/port/fm_port_time.c
    ${FM_SRC_DIR}/port/fm_port_threadx_idle.c
    ${FM_SRC_DIR}/port/fm_port_usart1.c
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${FM_APP_SOURCES}
    ${FM_COMMON_SOURCES}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${FM_APP_INCLUDES}
    ${FM_SRC_DIR}/apps
    ${FM_SRC_DIR}/libs
    ${FM_SRC_DIR}/bsp
    ${FM_SRC_DIR}/product/fmc
    ${FM_SRC_DIR}/services
    ${FM_SRC_DIR}/port
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
