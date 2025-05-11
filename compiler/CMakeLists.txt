cmake_minimum_required(VERSION 3.8)


include(FetchContent)
FetchContent_Declare(vtoml
    GIT_REPOSITORY https://github.com/viper-org/vtoml
    GIT_TAG master)
FetchContent_MakeAvailable(vtoml)

set(SOURCES
    "source/main.cpp"

    "source/Linker.cpp"
    "source/Builder.cpp"
)

set(HEADERS
    "include/Linker.h"
    "include/Builder.h"
)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_executable(basilisk ${SOURCES} ${HEADERS})
add_executable(basilisk::compiler ALIAS basilisk)

target_include_directories(basilisk PUBLIC include)
target_compile_features(basilisk PUBLIC cxx_std_20)
target_link_libraries(basilisk basilisk::framework)
target_link_libraries(basilisk vtoml)