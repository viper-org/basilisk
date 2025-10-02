cmake_minimum_required (VERSION 3.26)

project (basilisk)

add_subdirectory(framework)

add_subdirectory(compiler)

install(TARGETS basilisk DESTINATION bin)
install(DIRECTORY cmake/ DESTINATION cmake)