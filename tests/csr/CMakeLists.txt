# ----------------------------------------------------------------------------
# PIGO Test System
# Copyright (c) GT-TDAlab
#
# This contains the tests for COOs in PIGO
# ----------------------------------------------------------------------------

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/tests-bin)
set(CTEST_BINARY_DIRECTORY ${PROJECT_BINARY_DIR}/tests-bin)

file(GLOB test_sources "test_*.cpp")

foreach(src ${test_sources})
    get_filename_component(test_name ${src} NAME_WE)
    add_executable(${test_name} ${src})

    target_include_directories(${test_name} PUBLIC
        ${PROJECT_SOURCE_DIR}/tests)
    target_link_libraries(${test_name} pigo)
    target_link_libraries(${test_name} pigo_test_utils)
    target_link_libraries(${test_name} OpenMP::OpenMP_CXX)
    set_property(TARGET ${test_name} PROPERTY CXX_STANDARD 11)
    set_property(TARGET ${test_name} PROPERTY CXX_STANDARD_REQUIRED on)
    target_compile_options(${test_name} PRIVATE -Werror -Wall -Wextra)

    add_test(NAME ${test_name} COMMAND ${CTEST_BINARY_DIRECTORY}/${test_name}
        ${CMAKE_CURRENT_SOURCE_DIR})
    set_tests_properties(${test_name} PROPERTIES TIMEOUT 120)
endforeach()

file(GLOB time_sources "time_*.cpp")
file(GLOB test_inputs "data/*.el")

foreach(src ${time_sources})
    get_filename_component(time_name ${src} NAME_WE)
    add_executable(${time_name} ${src})

    target_link_libraries(${time_name} pigo)
    target_link_libraries(${time_name} OpenMP::OpenMP_CXX)
    set_property(TARGET ${time_name} PROPERTY CXX_STANDARD 11)
    set_property(TARGET ${time_name} PROPERTY CXX_STANDARD_REQUIRED on)
    target_compile_options(${time_name} PRIVATE -Werror -Wall -Wextra)
endforeach()
