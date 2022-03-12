cmake_minimum_required(VERSION 3.5)

aux_source_directory(. SRCS)
add_executable(server ${SRCS})
target_link_libraries(server PRIVATE ${LIBS})