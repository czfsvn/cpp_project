cmake_minimum_required(VERSION 3.5)

project(cpp_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE OFF) # print all detail of compiler options
set(CMAKE_CXX_EXTENSIONS OFF)



#SET(CMAKE_BUILD_TYPE "Debug")
#SET(CMAKE_BUILD_TYPE "Release")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)



# find the protobuf compiler and libraries
find_package(Protobuf REQUIRED)
# check if protobuf was found
if(PROTOBUF_FOUND)
    #message ("protobuf found")
else()
    #message (FATAL_ERROR "Cannot find Protobuf")
endif()

#
# 编译器选项
# -D CNCXX=gcc
# -D CNCXX=clang
# -D CNCXX=clang-lint
#

if (CNCXX STREQUAL gcc)
    set(CMAKE_CXX_COMPILER g++)
    set(CMAKE_C_COMPILER gcc)
elseif (CNCXX STREQUAL clang-lint)
    set(CMAKE_CXX_COMPILER c++-analyzer)
    set(CMAKE_C_COMPILER ccc-analyzer)
else()
    set(CMAKE_CXX_COMPILER clang++)
    set(CMAKE_C_COMPILER clang)
endif()

if(APPLE)
    #message("UNIX1")
    SET(INC_DIR /usr/local/include/ /usr/local/boost/boost_1_75_0)
    SET(LIB_DIR /usr/local/lib/ /usr/local/boost/boost_1_75_0/stage/lib)
elseif(WIN32)
    #message("WIN32")
elseif(UNIX)
    #message("UNIX3")
    SET(INC_DIR /usr/local/include /usr/boost_1_75/include ${PROTOBUF_INCLUDE_DIRS})
    SET(LIB_DIR /usr/local/lib /usr/boost_1_75/lib)
endif()

set(THIRD_LIBS 
    pthread
    protobuf
    boost_thread
    xml2
    )

set(LOCAL_LIB 
    cnbase 
    cnpb 
    cnnet
    cnxml
    )

set(LIBS ${THIRD_LIBS} ${LOCAL_LIB})

# -DCMAKE_DEBUG=OFF/ON  Release/Debug
option(CMAKE_DEBUG "Whether use debug type" OFF) # this type need to remove file CMakeCache.txt and re-cmake
#set(CMAKE_DEBUG OFF)

if(CMAKE_DEBUG) # 
    #string(APPEND C_OPTIONS " -Og")
    add_definitions (-D SPDLOG)
    add_definitions (-D SPDDEBUG)
    add_definitions (-D _DEBUG_)
    set(CMAKE_BUILD_TYPE "Debug")
else()
    # string(APPEND C_OPTIONS " -MD")
    #set(CMAKE_BUILD_TYPE "Release")
    #set(CMAKE_BUILD_TYPE "RelWithDebInfo")
    set(CMAKE_BUILD_TYPE "MinSizeRel")
    add_definitions (-D _RELEASE_)
endif()

add_definitions(-D FMT_HEADER_ONLY)

message(STATUS "--------Build type: [${CMAKE_BUILD_TYPE}] ------\n")
#message(STATUS "C flags, Debug configuration: ${CMAKE_C_FLAGS_DEBUG}")
#message(STATUS "C flags, Release configuration: ${CMAKE_C_FLAGS_RELEASE}")
#message(STATUS "C flags, Release configuration with Debug info: ${CMAKE_C_FLAGS_RELWITHDEBINFO}")
#message(STATUS "C flags, minimal Release configuration: ${CMAKE_C_FLAGS_MINSIZEREL}")
#message(STATUS "C++ flags, Debug configuration: ${CMAKE_CXX_FLAGS_DEBUG}")
#message(STATUS "C++ flags, Release configuration: ${CMAKE_CXX_FLAGS_RELEASE}")
#message(STATUS "C++ flags, Release configuration with Debug info: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
#message(STATUS "C++ flags, minimal Release configuration: ${CMAKE_CXX_FLAGS_MINSIZEREL}")


set(CXX_FLAGS
    #-fno-elide-constructors
    -fpermissive
    -Wno-deprecated
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS}")
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE) 
message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})


include_directories(${INC_DIR})
include_directories(${PROJECT_SOURCE_DIR}/msg)
include_directories(${PROJECT_SOURCE_DIR}/base)
include_directories(${PROJECT_SOURCE_DIR}/net)
include_directories(${PROJECT_SOURCE_DIR}/xml)
include_directories(${PROJECT_SOURCE_DIR}/3rd/cmdline)
include_directories(${PROJECT_SOURCE_DIR}/3rd/spdlog/include)
include_directories(${PROJECT_SOURCE_DIR}/3rd/libxml2/include)

link_directories(${LIB_DIR} 
    ${LIBRARY_OUTPUT_PATH} 
    /home/cn/work/cpp/cpp_project/build/lib/ 
    ${PROJECT_SOURCE_DIR}/3rd/libxml2/lib)

add_subdirectory("${PROJECT_SOURCE_DIR}/base")
add_subdirectory("${PROJECT_SOURCE_DIR}/msg")
add_subdirectory("${PROJECT_SOURCE_DIR}/net")
add_subdirectory("${PROJECT_SOURCE_DIR}/xml")


#add_subdirectory("${PROJECT_SOURCE_DIR}/tcpserver")
#add_subdirectory("${PROJECT_SOURCE_DIR}/tcpclient")

#add_subdirectory("${PROJECT_SOURCE_DIR}/examples")

set(SERVERS tcpserver tcpclient examples)

foreach(SERVER ${SERVERS})
    add_subdirectory(${SERVER})
endforeach(SERVER)

