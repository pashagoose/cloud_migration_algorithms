include_directories(${PROTOBUF_INCLUDE_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS test_case.proto)

add_library(proto_lib ${PROTO_HDRS} ${PROTO_SRCS})

target_link_libraries(proto_lib PUBLIC ${PROTOBUF_LIBRARY})