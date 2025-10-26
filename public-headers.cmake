# This file defines list of public headers.
# "Public headers" stands for headers exposing to clients.

# First list all headers here
file (GLOB_RECURSE THREAD_POOL_SORTER_PUBLIC_HEADERS_LIST RELATIVE "${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/src/*.h")

# Concat list items with semicolon to satisfy set_target_properties()
list (JOIN THREAD_POOL_SORTER_PUBLIC_HEADERS_LIST "\;" THREAD_POOL_SORTER_PUBLIC_HEADERS)

# Cleanup
unset (THREAD_POOL_SORTER_PUBLIC_HEADERS_LIST)
