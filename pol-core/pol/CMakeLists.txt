set(ex_name pol)

# define source files
include(CMakeSources.cmake)

add_executable(${ex_name}
  ${${ex_name}_sources}
)    
 
set_compile_flags(${ex_name} 1)
enable_pch(${ex_name})
use_curl(${ex_name})
use_benchmark(${ex_name})

target_link_libraries(${ex_name} PUBLIC
  bscript
  clib
  plib
  tinyxml
)
if (${linux})
  target_link_libraries(${ex_name} PUBLIC
    crypto
	pthread
	z
  )
endif()
if (${gcc})
  target_link_libraries(${ex_name} PUBLIC
	-fopenmp
  )
endif()

if(HAVE_MYSQL)
  target_link_libraries(${ex_name} PUBLIC
	mysqlclient
  )
endif()
if(DEFINED USE_BOOST_REGEX)
  target_link_libraries(${ex_name} PUBLIC
    boost_regex
  )
endif()

dist(${ex_name} .)
