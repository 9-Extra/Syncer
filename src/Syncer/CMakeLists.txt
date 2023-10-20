find_package(spdlog CONFIG REQUIRED)

set(TARGET_NAME Syncer)

add_library(${TARGET_NAME} STATIC)

if(MSVC)
  target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wno-gnu-anonymous-struct -pedantic)
endif()

file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.cpp *.h)
target_sources(${TARGET_NAME} PUBLIC ${sources})

target_include_directories(${TARGET_NAME} PUBLIC . ${PROJECT_SOURCE_DIR}/include/)

# # 操作系统库，只需要链接
target_link_libraries(${TARGET_NAME} PUBLIC "Rpcrt4.lib")
# 第三方的库
target_link_libraries(${TARGET_NAME} PUBLIC nlohmann_json::nlohmann_json proto-serialize spdlog::spdlog)

# 设置调试时的工作目录
set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_ROOT}/bin")