# Command to generate the escript module tables for pol and runecl targets
file(GLOB EM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/support/scripts/*.em)
set(OUT_FOLDER ${PROJECT_BINARY_DIR}/module_defs)

set(GENERATED_MODULE_HEADERS )
foreach(em ${EM_SOURCES})
  get_filename_component(em_name ${em} NAME_WE)
  set(GENERATED_MODULE_HEADERS ${GENERATED_MODULE_HEADERS};${OUT_FOLDER}/${em_name}.h)
endforeach()

add_custom_command(
  OUTPUT ${GENERATED_MODULE_HEADERS}
  COMMAND ${CMAKE_COMMAND}
    -DEM_FOLDER=${CMAKE_CURRENT_LIST_DIR}/support/scripts
    -DOUT_FOLDER=${OUT_FOLDER}
    -P ${PROJECT_SOURCE_DIR}/cmake/parse_modules.cmake
  DEPENDS ${EM_SOURCES};${PROJECT_SOURCE_DIR}/cmake/parse_modules.cmake
)

# The parse_module target runs the above command
add_custom_target(parse_modules
  DEPENDS ${GENERATED_MODULE_HEADERS}
)


set(POLCORE_DIR ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(clib)
add_subdirectory(bscript)
add_subdirectory(plib)

add_subdirectory(pol)
add_subdirectory(ecompile)
add_subdirectory(runecl)
add_subdirectory(uoconvert)
add_subdirectory(uotool)
add_subdirectory(poltool)

add_subdirectory(doc)
add_subdirectory(support)

