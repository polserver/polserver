###############################################################################

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeEnvironment.txt)

MESSAGE("####################################")
MESSAGE("##### Building POLTOOL")
MESSAGE("####################################")

###############################################################################
# define source files
SET (SOURCES
    PolToolMain.cpp
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
# link to the needed libs
SET (LIBS_LINUX
    clib
    plib
    z
)
SET (LIBS_LINUX_STATIC_BIN_STATIC
    clib_stic
    plib_stic
)
	
##############################################################
# define target directory
SET (TARGET_DIRECTORY
	${CMAKE_CURRENT_LIST_DIR}/../bin
)

##############################################################
# define binary name
SET (TARGET_PROGRAM_NAME
	poltool
)

##############################################################
# include CMake core
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CMakeCore.txt)
