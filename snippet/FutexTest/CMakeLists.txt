add_executable (FutexTest main.cpp)

target_compile_options (FutexTest PRIVATE "-fsanitize=address")
target_link_options (FutexTest PRIVATE "-fsanitize=address")
