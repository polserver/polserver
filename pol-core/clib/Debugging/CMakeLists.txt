###############################################################################

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../../../cmake/CMakeEnvironment.txt)

MESSAGE("####################################")
MESSAGE("##### Building POL - CLIB - DEBUGGING")
MESSAGE("####################################")

###############################################################################
# define source files
SET (SOURCES
	LogSink.cpp
	ExceptionParser.cpp		
)

##############################################################
# include dirs
SET (INCLUDE_DIRS
	./
	../..
)	

##############################################################
# define target directory
SET (TARGET_DIRECTORY
	${CMAKE_CURRENT_LIST_DIR}/../../bin
)

##############################################################
# define binary name
SET (TARGET_LIB_NAME
	clib_debugging
)

##############################################################
# include CMake core
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/CMakeCore.txt)

