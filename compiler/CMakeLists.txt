cmake_minimum_required(VERSION 3.8)

set(SOURCES
    "source/main.cpp"
    "source/Options.cpp"
)

set(HEADERS
    "include/Options.h"
)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_executable(basilisk ${SOURCES} ${HEADERS})
add_executable(basilisk::compiler ALIAS basilisk)

target_include_directories(basilisk PUBLIC include)
target_compile_features(basilisk PUBLIC cxx_std_20)
target_link_libraries(basilisk basilisk::framework)