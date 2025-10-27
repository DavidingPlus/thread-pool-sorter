add_executable (FileOpWithCacheDemo
    main.cpp
    lfile.cpp
    lcachemanager.cpp
)
target_link_libraries (FileOpWithCacheDemo thread-pool-sorter)
