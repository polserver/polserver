set (runecl_sources  # sorted !
  ${PROJECT_BINARY_DIR}/modtbl-runecl.cpp # Generated from 'parse_modules' custom target
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
  CMakeSources.cmake 
  RunEclMain.cpp
  RunEclMain.h
)
