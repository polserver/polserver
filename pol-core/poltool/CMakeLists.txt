set(ex_name poltool)

add_executable(${ex_name}
  # sorted !
  PolToolMain.cpp
)    

set_compile_flags(${ex_name} 1)

target_link_libraries(${ex_name} PUBLIC
  clib
  plib
  z
)
dist(${ex_name} .)
