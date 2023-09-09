
# Root path
set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

add_library(configor INTERFACE)
target_include_directories(configor INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

# add_subdirectory(examples examples)