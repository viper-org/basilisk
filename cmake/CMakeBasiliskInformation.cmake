set( CMAKE_Basilisk_COMPILE_OBJECT
    "<CMAKE_Basilisk_COMPILER> <FLAGS> -o <OBJECT> <SOURCE>"
)

set( CMAKE_Basilisk_LINK_EXECUTABLE 
    "gcc -o <TARGET> <OBJECTS>"
)

# TODO: Static and shared libraries

set(CMAKE_Basilisk_FLAGS_DEBUG 
    "-g -O0"
)
set(CMAKE_Basilisk_FLAGS_RELEASE 
    "-O3"
)

set(CMAKE_Basilisk_INFORMATION_LOADED 1)