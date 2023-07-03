cmake_minimum_required(VERSION 3.10)

include(GoogleTest)

file(GLOB_RECURSE DSBUS_TEST_SOURCES "${PROJECT_SOURCE_DIR}/test/*/*test.cpp")

add_custom_target(build-tests COMMAND ${CMAKE_CTEST_COMMAND} --show-only)
add_custom_target(check-tests COMMAND ${CMAKE_CTEST_COMMAND} --verbose)
add_custom_target(check-public-ci-tests COMMAND ${CMAKE_CTEST_COMMAND} --verbose -E SQLLogicTest)

foreach (dsbus_test_source ${DSBUS_TEST_SOURCES})
    # Create a human readable name.
    get_filename_component(dsbus_test_filename ${dsbus_test_source} NAME)
    string(REPLACE ".cpp" "" dsbus_test_name ${dsbus_test_filename})

    # Add the test target separately and as part of "make check-tests".
    add_executable(${dsbus_test_name} EXCLUDE_FROM_ALL ${dsbus_test_source})
    add_dependencies(build-tests ${dsbus_test_name})
    add_dependencies(check-tests ${dsbus_test_name})

    gtest_discover_tests(${dsbus_test_name}
            EXTRA_ARGS
            --gtest_color=auto
            --gtest_output=xml:${CMAKE_BINARY_DIR}/test/${dsbus_test_name}.xml
            --gtest_catch_exceptions=0
            DISCOVERY_TIMEOUT 120
            PROPERTIES
            TIMEOUT 120
            )

    target_link_libraries(${dsbus_test_name} dsbus gtest gmock_main)

    # Set test target properties and dependencies.
    set_target_properties(${dsbus_test_name}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
            COMMAND ${dsbus_test_name}
            )
endforeach ()