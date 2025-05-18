if(NOT DEFINED CMAKE_Basilisk_COMPILE_OBJECT)
    set(CMAKE_Basilisk_COMPILE_OBJECT
        "<CMAKE_Basilisk_COMPILER> <FLAGS> -o <OBJECT> <SOURCE>"
    )
endif()

if(NOT DEFINED CMAKE_Basilisk_LINK_EXECUTABLE)
    set(CMAKE_Basilisk_LINK_EXECUTABLE 
        "basilisk --cexec -o <TARGET> <OBJECTS>"
    )
endif()

if(NOT DEFINED CMAKE_Basilisk_ARCHIVE_CREATE)
    set(CMAKE_Basilisk_ARCHIVE_CREATE 
        "<CMAKE_AR> rc <TARGET> <OBJECTS>"
    )
endif()
if(NOT DEFINED CMAKE_Basilisk_ARCHIVE_APPEND)
    set(CMAKE_Basilisk_ARCHIVE_APPEND 
        "<CMAKE_AR> r <TARGET> <OBJECTS>"
    )
endif()
if(NOT DEFINED CMAKE_Basilisk_ARCHIVE_FINISH)
    set(CMAKE_Basilisk_ARCHIVE_FINISH 
        "<CMAKE_RANLIB> <TARGET>"
    )
endif()

# TODO: Shared libraries

set(CMAKE_Basilisk_FLAGS_DEBUG 
    "-g -O0"
)
set(CMAKE_Basilisk_FLAGS_RELEASE 
    "-O3"
)

set(CMAKE_Basilisk_INFORMATION_LOADED 1)