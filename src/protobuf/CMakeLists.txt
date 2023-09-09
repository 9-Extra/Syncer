find_package(Protobuf CONFIG REQUIRED)

set(TARGET_NAME proto-serialize)
add_library(${TARGET_NAME} OBJECT "${CMAKE_CURRENT_LIST_DIR}/proto/Serialize.proto")

target_link_libraries(${TARGET_NAME} PUBLIC protobuf::libprotobuf)

set(PROTO_BINARY_DIR "${CMAKE_CURRENT_LIST_DIR}/.gen/proto")

target_include_directories(${TARGET_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/.gen/proto>")

protobuf_generate(
    TARGET ${TARGET_NAME}
    IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}/proto"
    PROTOC_OUT_DIR "${PROTO_BINARY_DIR}"
)