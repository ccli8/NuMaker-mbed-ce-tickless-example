# Copyright (c) 2022 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19)
cmake_policy(VERSION 3.19)

set(MBED_OS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")

# Set default path for mbed_app.json5 (no force-write for override)
set(MBED_APP_JSON_PATH mbed_app.json5 CACHE INTERNAL "")

# Set default path for custom_targets.json5 (no force-write for override)
set(CUSTOM_TARGETS_PATH custom_targets CACHE INTERNAL "")
set(CUSTOM_TARGETS_JSON_PATH ${CUSTOM_TARGETS_PATH}/custom_targets.json5 CACHE INTERNAL "")

# Load Mbed CE toolchain file and basic build system
include(${MBED_OS_PATH}/tools/cmake/app.cmake)

set(APP_PROJECT NUMAKER_MBED_CE_TICKLESS_EXAMPLE)
set(APP_TARGET NuMaker-mbed-ce-tickless-example)

# Set up project name
project(${APP_PROJECT})

# Add Mbed OS library
add_subdirectory(${MBED_OS_PATH})

add_executable(${APP_TARGET})

target_sources(${APP_TARGET}
    PRIVATE
        main.cpp
        wakeup_button.cpp
        wakeup_i2c.cpp
        wakeup_pwrctl.cpp
        wakeup_rtc.cpp
        wakeup_uart.cpp
        wakeup_wdt.cpp
)

target_link_libraries(${APP_TARGET}
    PRIVATE
        mbed-os
)

# Must call this for each target to set up bin file creation, code upload, etc
mbed_set_post_build(${APP_TARGET})

# Make sure this is the last line of the top-level build script
mbed_finalize_build()
