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

#clib is our base lib, if this waits for boost extraction
#its enough, the external libs can start to build already
if(NOT EXISTS "${BOOST_SOURCE_DIR}/boost")
  add_dependencies(${lib_name} boost)
endif()

add_dependencies(${lib_name} git_rev)

target_link_libraries(${lib_name} PUBLIC
  format
)

if (${linux})
  target_link_libraries(${lib_name} PUBLIC
    crypto
    pthread
    atomic
  )
elseif(${windows})
  target_link_libraries(${lib_name} PUBLIC
	stackwalk
	ws2_32
	Psapi
	DbgHelp
#	crypto
  )
endif()

use_tidy(${lib_name})
