# ----------------------------------------------------------------------------
# PIGO
# Copyright (c) GT-TDAlab
# ----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.12...3.19)

project(
    PIGO
    VERSION 0.4
    LANGUAGES CXX
    DESCRIPTION "A parallel graph and matrix IO and preprocessing library"
    )

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# ----------------------------------------------------------------------------
# Require OpenMP packages
find_package(OpenMP REQUIRED)

# ----------------------------------------------------------------------------
# Create the pigo library target
add_library(pigo INTERFACE)
target_include_directories(pigo INTERFACE include/)

# ----------------------------------------------------------------------------
# Force out-of-source
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You likely want to build or install PIGO.
    Please build from a new, separate directory:
    `cd build; cmake ..; make install`.
    If you know what you are doing and intend to build in this directory,
    please modify CMakeLists.txt to remove this check.")
endif()

# ----------------------------------------------------------------------------
# Support documentation and tests if we are the main project
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # ------------------------------------------------------------------------
    # Default to a Release build
    if (NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
        if (NOT CMAKE_BUILD_TYPE)
            set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
        endif()
    endif()

    # ------------------------------------------------------------------------
    # Add in documentation as a target
    add_subdirectory(docs)

    # ------------------------------------------------------------------------
    # Enable ctest
    enable_testing()

    add_subdirectory(tests)
endif()
