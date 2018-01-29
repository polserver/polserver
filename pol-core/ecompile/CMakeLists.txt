set(ex_name ecompile)

add_executable(${ex_name}
  # sorted !
  ECompileMain.cpp
)    
 
set_compile_flags(${ex_name} 1)

target_link_libraries(${ex_name} PUBLIC
  bscript
  clib
  plib
  pthread    
)
if (${gcc})
  target_link_libraries(${ex_name} PUBLIC
    -fopenmp
  )
endif()
dist(${ex_name} scripts)
