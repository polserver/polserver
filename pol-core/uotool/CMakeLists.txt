set(ex_name uotool)

add_executable(${ex_name}
  # sorted !
  ../pol/globals/multidefs.cpp
  ../pol/multi/multidef.cpp
  ../pol/polfile1.cpp
  UoToolMain.cpp
)    

set_compile_flags(${ex_name} 1)

target_link_libraries(${ex_name} PRIVATE
  clib
  plib
  pthread    
)

dist(${ex_name} .)
