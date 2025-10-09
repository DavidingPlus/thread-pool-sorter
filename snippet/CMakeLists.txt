# Collect snippets marked as being built
file (GLOB ITEMS RELATIVE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/*)
set (SUB_DIRS "")
foreach (ITEM ${ITEMS})
    if (
        IS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${ITEM}
        AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/${ITEM}/CMakeLists.txt
        AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/${ITEM}/.buildme
    )
        list (APPEND SUB_DIRS ${ITEM})

    endif()

endforeach()

# Run add_subdirectory() for each
foreach (SUB_DIR ${SUB_DIRS})
    message ("-- [Thread Pool Sorter] Building snippet: ${SUB_DIR}")
    add_subdirectory (${SUB_DIR})

endforeach()

message ("-- [Thread Pool Sorter] Done configuring snippets")
