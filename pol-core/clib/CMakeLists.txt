set(lib_name clib)

# define source files
include(CMakeSources.cmake)

add_library(${lib_name} STATIC
  ${${lib_name}_sources}
)
set_compile_flags(${lib_name} 0)
warning_suppression(${lib_name})
enable_pch(${lib_name})

use_benchmark(${lib_name})

add_dependencies(${lib_name} git_rev)

target_link_libraries(${lib_name} PUBLIC
  libfmt
  libutf8
  libboost_headers
  #  libboost_regex
  #  libboost_system
  #  libboost_thread
)

if (${linux})
  if (APPLE)
    pkg_search_module(OPENSSL QUIET openssl)
    target_include_directories(${lib_name} PRIVATE ${OPENSSL_INCLUDEDIR})
    target_link_directories(${lib_name} PRIVATE ${OPENSSL_LIBDIR})
  endif()
  target_link_libraries(${lib_name} PRIVATE
    crypto
    pthread
  )
  if (arm_proc AND NOT APPLE)
    target_link_libraries(${lib_name} PRIVATE atomic)
  endif()
elseif(${windows})
  target_link_libraries(${lib_name} PRIVATE
    libstackwalk
    ws2_32
    Psapi
    DbgHelp
  )
endif()

use_tidy(${lib_name})
