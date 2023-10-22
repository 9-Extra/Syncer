set(TARGET_NAME Test)

add_executable(${TARGET_NAME})

file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.cpp *.h)
target_sources(${TARGET_NAME} PRIVATE ${sources})

target_include_directories(${TARGET_NAME} PUBLIC .)

# add_definitions(-DSYNCERIMPL)
# # 操作系统库，只需要链接
# target_link_libraries(${TARGET_NAME} PUBLIC "icu.lib")
# 第三方的库
target_link_libraries(${TARGET_NAME} PUBLIC Syncer)

# 设置调试时的工作目录
set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_ROOT}/bin")