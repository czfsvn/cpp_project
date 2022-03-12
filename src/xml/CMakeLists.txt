
cmake_minimum_required(VERSION 3.5)

aux_source_directory(. SRCS)
add_library(cnxml STATIC ${SRCS})

target_link_libraries(cnxml ${THIRD_LIBS})