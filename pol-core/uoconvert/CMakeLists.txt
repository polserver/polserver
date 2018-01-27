set(ex_name uoconvert)

add_executable(${ex_name}
  # sorted !
  ../pol/polfile1.cpp
  UoConvertMain.cpp
)    
 
set_compile_flags(${ex_name} 1)

target_link_libraries(${ex_name} PRIVATE
  clib
  plib
  pthread    
)

dist(${ex_name} .)
