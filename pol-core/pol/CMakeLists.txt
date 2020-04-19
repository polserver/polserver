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
use_curl(${ex_name})
use_benchmark(${ex_name})

target_link_libraries(${ex_name} PUBLIC
  bscript
  clib
  plib
  tinyxml
  z
)
if (${linux})
  target_link_libraries(${ex_name} PUBLIC
    crypto
    pthread
  )
endif()

if(HAVE_MYSQL)
  if(${windows})
    if (${ARCH_BITS} EQUAL "64")
      set(mysql_path ${CMAKE_CURRENT_LIST_DIR}/../../lib/mysql-connector-c-6.0.2-winx64)
    else()
      set(mysql_path ${CMAKE_CURRENT_LIST_DIR}/../../lib/mysql-connector-c-6.0.2-win32)
    endif()

    target_include_directories(${ex_name} PUBLIC
      ${mysql_path}/include
    )
    target_link_libraries(${ex_name} PUBLIC
      ${mysql_path}/lib/libmysql.lib
    )

    add_custom_command(TARGET ${ex_name} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${mysql_path}/lib/libmysql.dll"
        "$<TARGET_FILE_DIR:${ex_name}>/libmysql.dll"
    )

    install(
      FILES ${mysql_path}/lib/libmysql.dll
      DESTINATION .
      COMPONENT bin
    )
  else()
    target_link_libraries(${ex_name} PUBLIC
      mysqlclient
    )
  endif()
endif()

dist(${ex_name} .)

use_tidy(${ex_name})
