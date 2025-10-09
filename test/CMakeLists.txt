find_package (GTest REQUIRED)

aux_source_directory (${CMAKE_CURRENT_SOURCE_DIR} SRC_GTEST)
add_executable (gtest-thread-pool-sorter EXCLUDE_FROM_ALL ${SRC_GTEST})
target_link_libraries (gtest-thread-pool-sorter gtest::gtest thread-pool-sorter)

add_custom_target (tests)
add_dependencies (tests gtest-thread-pool-sorter)


message ("-- [Thread Pool Sorter] Done configuring gtest")
