cmake_minimum_required(VERSION 3.5)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/examples/bin)

add_executable(examples main.cpp boost_asio.cpp)
target_link_libraries(examples ${LIBS})

add_executable(testproto prototest.cpp)
target_link_libraries(testproto ${LIBS})

add_executable(testmemory memorytest.cpp)
target_link_libraries(testmemory ${LIBS})

add_executable(testthreadpool testthreadpool.cpp)
target_link_libraries(testthreadpool ${LIBS})