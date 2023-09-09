# 学习HEngine的CMakeLists
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if (WIN32)
    add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
endif()

if (MSVC)
    add_compile_options("/MP")
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Syncer)
endif()

add_subdirectory(protobuf)
add_subdirectory(Syncer)
add_subdirectory(Test)