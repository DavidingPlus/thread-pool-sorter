find_package (GTest REQUIRED)

aux_source_directory (${CMAKE_CURRENT_SOURCE_DIR} SRC_GTEST)
add_executable (gtest-testrun EXCLUDE_FROM_ALL ${SRC_GTEST})
target_link_libraries (gtest-testrun gtest::gtest linux-environment-advanced-programming)

add_custom_target (tests)
add_dependencies (tests gtest-testrun)


message ("-- [Linux Environment Advanced Programming] Done configuring gtest")
