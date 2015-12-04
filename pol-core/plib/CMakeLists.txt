###############################################################################

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeEnvironment.txt)

MESSAGE("####################################")
MESSAGE("##### Building POL - PLIB")
MESSAGE("####################################")

###############################################################################
# define source files
SET (SOURCES  # sorted !
	../pol/uofile00.cpp       # TODO: move all uofile*.cpp into the plib folder
	../pol/uofile01.cpp
	../pol/uofile02.cpp
	../uotool/uofile04.cpp 
	../uotool/uofile05.cpp
	../pol/uofile06.cpp
	../pol/uofile07.cpp
	../pol/uofile08.cpp
	filemapserver.cpp 
	inmemorymapserver.cpp
	mapfunc.cpp 
	mapserver.cpp 
	maptileserver.cpp 
	mapwriter.cpp
	pkg.cpp 
	realmdescriptor.cpp 
	staticserver.cpp
	systemstate.cpp
	testdrop1.cpp 
	testenv.cpp
	testlos1.cpp 
	testwalk1.cpp
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

