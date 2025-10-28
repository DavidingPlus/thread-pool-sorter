add_executable (CachedFileOpsDemo
    main.cpp
    lfile.cpp
    lcachemanager.cpp
)
target_link_libraries (CachedFileOpsDemo thread-pool-sorter)
