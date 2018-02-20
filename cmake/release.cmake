macro(release)
  string(TIMESTAMP curr_date "%Y-%m-%d")
  if (${linux})
	set(system LINUX)
  else()
	set(system WINDOWS)
  endif()
  set(CPACK_PACKAGE_FILE_NAME "polserver${POL_VERSION_STR}-${system}-${curr_date}") 

  set(CPACK_PACKAGE_DIRECTORY "${PROJECT_BINARY_DIR}/../bin")
  set(CPACK_GENERATOR "ZIP")
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
    COMPONENT polcore
  )
endfunction()
