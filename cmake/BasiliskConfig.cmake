add_executable(basilisk IMPORTED)
find_program(BASILISK_EXEC_PATH basilisk HINTS "${CMAKE_CURRENT_LIST_DIR}/../../")
set_target_properties(basilisk PROPERTIES IMPORTED_LOCATION "${BASILISK_EXEC_PATH}")
LIST( APPEND CMAKE_MODULE_PATH ${Basilisk_DIR} )