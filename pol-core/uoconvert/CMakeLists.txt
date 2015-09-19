###############################################################################

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeConfig.txt)

MESSAGE("####################################")
MESSAGE("##### Building UOCONVERT")
MESSAGE("####################################")

###############################################################################
# define source files
SET (SOURCES  # sorted !
	../plib/mapwriter.cpp
	../pol/polfile1.cpp
	../pol/uofile00.cpp 
	../pol/uofile01.cpp
	../pol/uofile02.cpp
	../pol/uofile07.cpp
	../pol/uofile08.cpp
	uoconvert.cpp
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
    bscript
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
    uoconvert
)

##############################################################
# include CMake core
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CMakeCore.txt)
