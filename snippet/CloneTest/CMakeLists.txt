add_executable (CloneProcessTest main1.cpp)

# 三个修饰符
# PRIVATE：表示选项仅对当前目标生效，不会传播给依赖当前目标的其他目标。
# PUBLIC：表示选项不仅对当前目标生效，还会传播给依赖当前目标的其他目标。
# INTERFACE：表示选项不会直接作用于当前目标，但会传播给依赖当前目标的其他目标。
target_compile_options (CloneProcessTest PRIVATE "-fsanitize=address")
target_link_options (CloneProcessTest PRIVATE "-fsanitize=address")


add_executable (CloneThreadTest main2.cpp)

target_compile_options (CloneThreadTest PRIVATE "-fsanitize=address")
target_link_options (CloneThreadTest PRIVATE "-fsanitize=address")
