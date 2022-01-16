cmake_minimum_required(VERSION 3.5)

aux_source_directory(. SRCS)

add_library(cnbase STATIC ${SRCS})