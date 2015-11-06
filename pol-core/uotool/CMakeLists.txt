###############################################################################

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeEnvironment.txt)

MESSAGE("####################################")
MESSAGE("##### Building UOTOOL")
MESSAGE("####################################")

###############################################################################
# define source files
SET (SOURCES  # sorted !
	../pol/globals/multidefs.cpp
	../pol/multi/multidef.cpp 
	../pol/polfile1.cpp
	../pol/uofile00.cpp
	../pol/uofile01.cpp
	../pol/uofile02.cpp
	../pol/uofile06.cpp
	../pol/uofile07.cpp
	../pol/uofile08.cpp
	uofile04.cpp 
	uofile05.cpp
	uotool.cpp
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
    clib
    plib
)
SET (LIBS_LINUX_STATIC_BIN_STATIC
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
    uotool
)

##############################################################
# include CMake core
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CMakeCore.txt)
