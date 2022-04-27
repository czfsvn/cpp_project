cmake_minimum_required(VERSION 3.5)

#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/examples/bin)

set(SKIPSRC 
    MySkipList/skipmain.cpp 
    MySkipList/skiplist.cpp 
    MySkipList/TopList.cpp 
    MySkipList/skiplist2.cpp
    MySkipList/skiplist4.cpp
    )

set(TIMESRC 
    TimeUtils/timetest.cpp
    )

set(XML 
    Xml/xmltest.cpp
    )

set(FMT 
    fmt/fmt.cpp
    )

set(CONVERT typeconvert/typeconvert.cpp)
set(URI uriparser/test_uri_parser.cpp)
set(ENCRYPT encrypt/test_encrypt.cpp)
set(ALGOROTHM algorithm/algorithm.cpp)


aux_source_directory(. SRCS)

set(TARGET example)




add_executable(${TARGET} 
    ${SRCS}
    ${SKIPSRC} 
    ${TIMESRC}
    ${XML}
    ${CONVERT}
    ${URI}
    ${ENCRYPT}
    ${FMT}
    ${ALGOROTHM}
    )

target_link_libraries(${TARGET} ${LIBS})

#add_executable(testproto prototest.cpp)
#target_link_libraries(testproto ${LIBS})

#add_executable(testmemory memorytest.cpp)
#target_link_libraries(testmemory ${LIBS})

#add_executable(testthreadpool testthreadpool.cpp)
#target_link_libraries(testthreadpool ${LIBS})

#add_executable(testcmdlines testcmdlines.cpp)
#target_link_libraries(testcmdlines ${LIBS})

#add_executable(testspd testspd.cpp)
#target_link_libraries(testspd ${LIBS})

#add_executable(testxml2 testxml2.cpp)
#target_link_libraries(testxml2 ${LIBS})

#add_executable(example example.cpp)
#target_link_libraries(example ${LIBS})