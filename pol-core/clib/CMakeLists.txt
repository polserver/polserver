set(lib_name clib)

# define source files
include(CMakeSources.cmake)

add_library(${lib_name} STATIC
  ${${lib_name}_sources}
)
set_compile_flags(${lib_name} 0)

use_benchmark(${lib_name})

if (${linux})
  target_link_libraries(${lib_name} PUBLIC
	crypto
    pthread
  )
elseif(${windows})
  target_link_libraries(${lib_name} PUBLIC
	ws2_32
	Psapi
	DbgHelp
#	crypto
  )
endif()

