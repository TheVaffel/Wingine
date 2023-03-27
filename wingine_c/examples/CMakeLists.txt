add_executable(trivial trivial.c)
add_executable(triangle triangle.c)

link_directories(../build)

target_link_libraries(trivial wingine_c)
target_link_libraries(triangle wingine_c)
