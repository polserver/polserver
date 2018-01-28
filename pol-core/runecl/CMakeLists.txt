set(ex_name runecl)

add_executable(${ex_name}
  # sorted !
  ../pol/binaryfilescrobj.cpp 
  ../pol/cfgrepos.cpp 
  ../pol/dice.cpp
  ../pol/globals/ucfg.cpp
  ../pol/module/basiciomod.cpp 
  ../pol/module/basicmod.cpp 
  ../pol/module/cfgmod.cpp 
  ../pol/module/datastore.cpp
  ../pol/module/filemod.cpp 
  ../pol/module/mathmod.cpp
  ../pol/module/utilmod.cpp 
  ../pol/proplist.cpp
  ../pol/xmlfilescrobj.cpp
  RunEclMain.cpp
)    
 
set_compile_flags(${ex_name} 1)

target_link_libraries(${ex_name} PUBLIC
  bscript
  clib
  plib
  tinyxml
  pthread
)

dist(${ex_name} scripts)
