###############################################################################

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeConfig.txt)

MESSAGE("####################################")
MESSAGE("##### Building UOTOOL")
MESSAGE("####################################")

###############################################################################
# define source files
SET (SOURCES
    uotool.cpp
	uofile04.cpp 
	uofile05.cpp
    ../../lib/format/format.cc
	../pol/uofile00.cpp
	../pol/uofile01.cpp
	../pol/uofile02.cpp
	../pol/uofile06.cpp
	../pol/uofile07.cpp
	../pol/uofile08.cpp
	../pol/polfile1.cpp
	../pol/multi/multidef.cpp 
	../pol/globals/multidefs.cpp
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
    uotool
)

##############################################################
# include CMake core
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CMakeCore.txt)
