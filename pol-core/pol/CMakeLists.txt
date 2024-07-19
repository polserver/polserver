set(ex_name pol)

# define source files
include(CMakeSources.cmake)

add_executable(${ex_name}
  ${${ex_name}_sources}
)

add_modtbl_dependency(${ex_name})

set_compile_flags(${ex_name} 1)
warning_suppression(${ex_name})
enable_pch(${ex_name})
use_benchmark(${ex_name})

target_link_libraries(${ex_name} PUBLIC
  bscript
  clib
  libcppdap
  plib
  libtinyxml
  libz
  libpicojson
  libcurl
  libsqlitecpp1
  libsqlitecpp2
)

if(HAVE_MYSQL)
  target_link_libraries(${ex_name} PUBLIC libsql)

  if(${windows})
    get_target_property(sqldll libsql IMPORTED_LOCATION)
    get_filename_component(dllname ${sqldll} NAME)
    add_custom_command(TARGET ${ex_name} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${sqldll}"
        "$<TARGET_FILE_DIR:${ex_name}>/${dllname}"
    )
    install(
      FILES ${sqldll}
      DESTINATION .
      COMPONENT bin
    )
  endif()
endif()

dist(${ex_name} .)

use_tidy(${ex_name})
