add_executable (CachedFileOpsDemo
    main.cpp
    lcachedfile.cpp
    lcachemanager.cpp
)
target_link_libraries (CachedFileOpsDemo thread-pool-sorter)
