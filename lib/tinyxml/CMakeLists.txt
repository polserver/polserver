###############################################################################

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeConfig.txt)

MESSAGE("####################################")
MESSAGE("##### Building POL - TINYXML")
MESSAGE("####################################")

###############################################################################
# define source files
SET (SOURCES  # sorted !
    ../../lib/tinyxml/tinystr.cpp 
    ../../lib/tinyxml/tinyxml.cpp
    ../../lib/tinyxml/tinyxmlerror.cpp 
    ../../lib/tinyxml/tinyxmlparser.cpp 
)

##############################################################
# include dirs
SET (INCLUDE_DIRS
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
)

##############################################################
# define target directory
SET (TARGET_DIRECTORY
	${CMAKE_CURRENT_LIST_DIR}/../bin
)

##############################################################
# define binary name
SET (TARGET_LIB_NAME
	tinyxml
)

##############################################################
# include CMake core
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CMakeCore.txt)

