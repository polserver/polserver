set(lib_name docs)
file(GLOB_RECURSE docs_sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.*)

add_custom_target(${lib_name}
WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
VERBATIM
SOURCES ${docs_sources}
)

source_group_by_folder(${lib_name})

set_target_properties (${lib_name} PROPERTIES FOLDER Misc)
