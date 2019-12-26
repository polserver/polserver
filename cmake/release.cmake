macro(release)
  string(TIMESTAMP curr_date "%Y-%m-%d")
  if (${linux})
    set(system LINUX)
  else()
    set(system WINDOWS)
  endif()
  set(CPACK_PACKAGE_FILE_NAME "polserver${POL_VERSION_STR}-${system}-${curr_date}") 
  set(CPACK_COMPONENTS_GROUPING IGNORE)
  set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
  set(CPACK_PACKAGE_DIRECTORY ${output_bin_dir})
  set(CPACK_GENERATOR "ZIP")
  if (${linux})
    set(CPACK_STRIP_FILES ON)
  endif()
  include(CPack)
endmacro()

function(dist target dir)
# install target for our archive
# dir is the folder in our release zip
  install(
    TARGETS ${target} 
    ARCHIVE DESTINATION ${dir}
    LIBRARY DESTINATION ${dir}
    RUNTIME DESTINATION ${dir}
    COMPONENT bin
  )
  if (${windows})
    install(
      FILES $<TARGET_PDB_FILE:${target}>
      DESTINATION .
      COMPONENT dbg
    )
  endif()
endfunction()

function(add_polrelease_target)
  add_custom_target(
    PolRelease
    COMMAND "${CMAKE_COMMAND}" --build . --target clean --config Release
    COMMAND "${CMAKE_COMMAND}" --build . --target package --config Release
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Building Pol Release"
  )
  set_target_properties(PolRelease PROPERTIES EXCLUDE_FROM_ALL TRUE)
  set_target_properties(PolRelease PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
  set_target_properties(PolRelease PROPERTIES FOLDER !BuildTargets)

  if (${linux})
    add_custom_target(create_dbg_zip COMMAND
      ${CMAKE_COMMAND} -E tar "cfv" "${CPACK_PACKAGE_FILE_NAME}_dbg.zip" --format=zip
        "pol"
        "ecompile"
        "runecl"
        "uoconvert"
        "poltool"
        "uotool"
      WORKING_DIRECTORY ${output_bin_dir}
    )
  endif()
endfunction()
