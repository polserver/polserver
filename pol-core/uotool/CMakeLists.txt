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
   UoToolMain.cpp
)    

##############################################################
# include dirs
SET (INCLUDE_DIRS
)

##############################################################
# compile flags
IF(NOT WINDOWS)
	IF(GCC)
    	SET (FLAGS
	        ${FLAGS}
			-Wl,-R.
			-Wl,-R./lib
			-Wl,-R../lib
			-Wl,-R../../lib
	    )
    ENDIF()
ENDIF()

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
    pthread    
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

IF(NOT EXISTS "${BOOST_SOURCE_DIR}/boost")
ADD_DEPENDENCIES(uotool boost)
ENDIF()
