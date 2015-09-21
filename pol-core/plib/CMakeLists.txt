###############################################################################

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeConfig.txt)

MESSAGE("####################################")
MESSAGE("##### Building POL - PLIB")
MESSAGE("####################################")

###############################################################################
# define source files
SET (SOURCES  # sorted !
	filemapserver.cpp 
	inmemorymapserver.cpp
	mapfunc.cpp 
	mapserver.cpp 
	maptileserver.cpp 
	pkg.cpp 
	polver.cpp
	realmdescriptor.cpp 
	staticserver.cpp
	systemstate.cpp
	testdrop1.cpp 
	testenv.cpp
	testlos1.cpp 
	testwalk1.cpp
	../pol/multi/multicr.cpp
	../pol/ufunc.cpp
)

##############################################################
# include dirs
SET (INCLUDE_DIRS
	../..
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
	clib
)

##############################################################
# define target directory
SET (TARGET_DIRECTORY
	${CMAKE_CURRENT_LIST_DIR}/../bin
)

##############################################################
# define binary name
SET (TARGET_LIB_NAME
	plib
)

##############################################################
# include CMake core
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CMakeCore.txt)

