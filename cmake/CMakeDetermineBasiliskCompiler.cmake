find_program(
    CMAKE_Basilisk_COMPILER
    NAMES "basilisk"
    DOC "Basilisk Compiler"
)
message(STATUS ${CMAKE_Basilisk_COMPILER})

mark_as_advanced(CMAKE_Basilisk_COMPILER)

set(CMAKE_Basilisk_SOURCE_FILE_EXTENSIONS bslk;)

SET(CMAKE_Basilisk_OUTPUT_EXTENSION .o)
SET(CMAKE_Basilisk_COMPILER_ENV_VAR "")

configure_file(${CMAKE_CURRENT_LIST_DIR}/CMakeBasiliskCompiler.cmake.in
    ${CMAKE_PLATFORM_INFO_DIR}/CMakeBasiliskCompiler.cmake)