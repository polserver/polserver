###############################################################################

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeConfig.txt)

MESSAGE("####################################")
MESSAGE("##### Building RUNECL")
MESSAGE("####################################")

###############################################################################
# define source files
SET (SOURCES
    runecl.cpp
    ../../lib/tinyxml/tinystr.cpp
    ../../lib/tinyxml/tinyxml.cpp
    ../../lib/tinyxml/tinyxmlerror.cpp 
    ../../lib/tinyxml/tinyxmlparser.cpp
    ../../lib/format/format.cc
    ../pol/module/basiciomod.cpp 
    ../pol/module/basicmod.cpp 
    ../pol/module/utilmod.cpp 
    ../pol/dice.cpp
    ../pol/module/filemod.cpp 
    ../pol/binaryfilescrobj.cpp 
    ../pol/xmlfilescrobj.cpp
    ../pol/module/cfgmod.cpp 
    ../pol/module/datastore.cpp
    ../pol/cfgrepos.cpp 
    ../pol/proplist.cpp
    ../pol/globals/ucfg.cpp
    ../pol/module/mathmod.cpp
)    

##############################################################
# include dirs
SET (INCLUDE_DIRS
    .
    ../
)

##############################################################
# set directory for internal libs
SET (LIB_DIRS
    ${LIB_DIRS}
    ${CMAKE_CURRENT_LIST_DIR}/../bin
)
    
##############################################################
# link to the needed libs
SET (LIBS_LINUX
    clib_debugging
    clib
    plib
)

##############################################################
# define target directory
SET (TARGET_DIRECTORY
    ${CMAKE_CURRENT_LIST_DIR}/../bin
)

##############################################################
# define binary name
SET (TARGET_PROGRAM_NAME
    runecl
)

##############################################################
# include CMake core
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CMakeCore.txt)
