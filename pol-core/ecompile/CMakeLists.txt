###############################################################################

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeEnvironment.txt)

MESSAGE("####################################")
MESSAGE("##### Building ECOMPILE")
MESSAGE("####################################")

###############################################################################
# define source files
SET (SOURCES
    ecompile.cpp
)    

##############################################################
# include dirs
SET (INCLUDE_DIRS
    ../
    ./
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
    bscript
    clib
    plib
)
SET (LIBS_LINUX_STATIC_BIN_STATIC
    bscript_static
    clib_static
    plib_static
)

##############################################################
# define target directory
SET (TARGET_DIRECTORY
    ${CMAKE_CURRENT_LIST_DIR}/../bin
)

##############################################################
# define binary name
SET (TARGET_PROGRAM_NAME
    ecompile
)

##############################################################
# include CMake core
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CMakeCore.txt)
