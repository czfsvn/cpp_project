cmake_minimum_required(VERSION 3.5)

set(PROTOC "/usr/local/bin/protoc")

set(PROTOS_DIR          ${CMAKE_CURRENT_SOURCE_DIR})
set(PROTOS_CPPOUT_DIR   ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE PROTO_FILE *.proto)

foreach(PB ${PROTO_FILE})
    get_filename_component(PB_NAME ${PB} NAME_WE)
    get_filename_component(PB_PATH ${PB} PATH)

    set(PB_H        ${PB_NAME}.pb.h)
    set(PB_CPP      ${PB_NAME}.pb.cc)

    set(PB_CPPOUT   ${PROTOS_CPPOUT_DIR}/${PB_H}    ${PROTOS_CPPOUT_DIR}/${PB_CPP})

    set(PB_CPPOUTS  ${PB_CPPOUTS}   ${PB_CPPOUT})

    add_custom_command(OUTPUT ${PB_CPPOUT}
        COMMAND ${PROTOC} -I${PROTOS_DIR} --cpp_out=${PROTOS_DIR} --proto_path=${PB_PATH} ${PB}
        DEPENDS ${PB})

endforeach(PB ${PROTO_FILE})

#message("LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
#message("PB_CPPS: ${PROTOBUF_LIBRARIES}")

add_library(cnpb STATIC ${PB_CPPOUTS})
target_link_libraries(cnpb ${THIRD_LIBS})

