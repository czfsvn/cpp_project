cmake_minimum_required(VERSION 3.5)

project(cpp_project)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_VERBOSE_MAKEFILE OFF) # print all detail of compiler options

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# find the protobuf compiler and libraries
find_package(Protobuf REQUIRED)
# check if protobuf was found
if(PROTOBUF_FOUND)
    message ("protobuf found")
else()
    message (FATAL_ERROR "Cannot find Protobuf")
endif()


if(APPLE)
    message("UNIX1")
    SET(INC_DIR /usr/local/include/ /usr/local/boost/boost_1_75_0)
    SET(LIB_DIR /usr/local/lib/ /usr/local/boost/boost_1_75_0/stage/lib)
elseif(WIN32)
    message("WIN32")
elseif(UNIX)
    message("UNIX3")
    SET(INC_DIR /usr/local/include /usr/boost_1_75/include ${PROTOBUF_INCLUDE_DIRS})
    SET(LIB_DIR /usr/local/lib /usr/boost_1_75/lib)
endif()

set(THIRD_LIBS 
    pthread
    protobuf
    boost_thread
    )

set(LOCAL_LIB 
    cnbase 
    cnpb 
    #cnnet
    )

set(LIBS ${THIRD_LIBS} ${LOCAL_LIB})

SET(CMAKE_BUILD_TYPE "Debug")
#SET(CMAKE_BUILD_TYPE "Release")

include_directories(${INC_DIR})
include_directories(${PROJECT_SOURCE_DIR}/msg)
include_directories(${PROJECT_SOURCE_DIR}/base)

link_directories(${LIB_DIR} ${LIBRARY_OUTPUT_PATH} /home/cn/work/cpp/cpp_project/build/lib/)

add_subdirectory("${PROJECT_SOURCE_DIR}/base")
add_subdirectory("${PROJECT_SOURCE_DIR}/msg")
add_subdirectory("${PROJECT_SOURCE_DIR}/net")

add_subdirectory("${PROJECT_SOURCE_DIR}/examples")

add_subdirectory("${PROJECT_SOURCE_DIR}/tcpserver")
add_subdirectory("${PROJECT_SOURCE_DIR}/tcpclient")
