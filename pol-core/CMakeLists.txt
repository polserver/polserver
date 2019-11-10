add_subdirectory(clib)
add_subdirectory(bscript)
add_subdirectory(plib)

if (BUILD_ALL OR ONLY_POL)
  add_subdirectory(pol)
endif()
if (BUILD_ALL OR ONLY_ECOMPILE)
  add_subdirectory(ecompile)
endif()
if (BUILD_ALL OR ONLY_RUNECL)
  add_subdirectory(runecl)
endif()
if (BUILD_ALL OR ONLY_UOCONVERT)
  add_subdirectory(uoconvert)
endif()
if (BUILD_ALL OR ONLY_UOTOOL)
  add_subdirectory(uotool)
endif()
if (BUILD_ALL OR ONLY_POLTOOL)
  add_subdirectory(poltool)
endif()

add_subdirectory(doc)
add_subdirectory(support)

# Command to generate the escript module tables for pol and runecl targets
if (BUILD_ALL OR ONLY_POL OR ONLY_RUNECL)
  file(GLOB EM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/support/scripts/*.em)

  add_custom_command(
    OUTPUT ${PROJECT_BINARY_DIR}/modtbl-pol.cpp ${PROJECT_BINARY_DIR}/modtbl-runecl.cpp
    COMMAND ${CMAKE_COMMAND}
      -DEM_FOLDER=${CMAKE_CURRENT_LIST_DIR}/support/scripts
      -DOUT_FOLDER=${PROJECT_BINARY_DIR}
      -P ${PROJECT_SOURCE_DIR}/cmake/parse_modules.cmake
    DEPENDS ${EM_SOURCES};${PROJECT_SOURCE_DIR}/cmake/parse_modules.cmake

  )

  # The parse_module target runs the above command
  add_custom_target(parse_modules
    DEPENDS ${PROJECT_BINARY_DIR}/modtbl-pol.cpp ${PROJECT_BINARY_DIR}/modtbl-runecl.cpp
  )
endif()
