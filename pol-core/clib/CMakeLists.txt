###############################################################################

ADD_SUBDIRECTORY(Debugging)

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeConfig.txt)

MESSAGE("####################################")
MESSAGE("##### Building POL - CLIB")
MESSAGE("####################################")

###############################################################################
# define source files
SET (SOURCES
	clib_utils.cpp 
	boostutils.cpp 
	timer.cpp
	streamsaver.cpp
	progver.cpp 
	socketsvc.cpp 
	binaryfile.cpp 
	cfgfile.cpp 
	cfgsect.cpp
	dirlist.cpp
	fileutil.cpp 
	iohelp.cpp
	kbhit.cpp
	mlog.cpp 
	MD5.cpp
	opnew.cpp
	passert.cpp
	sckutil.cpp 
	strexcpt.cpp 
	strset.cpp 
	strutil.cpp
	threadhelp.cpp
	tracebuf.cpp
	fdump.cpp 
	stracpy.cpp
	random.cpp 
	cmdargs.cpp 
	wallclock.cpp
	wnsckt.cpp
	xmain.cpp
	unicode.cpp
	pkthelper.cpp
	logfacility.cpp
	esignal.cpp
)

##############################################################
# include dirs
SET (INCLUDE_DIRS
	/usr/local/include/stlport
	../../lib/boost_1_55_0
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
# compile flags
IF(NOT WINDOWS)
	SET (FLAGS
		${FLAGS}
		-fno-strict-aliasing  
		-Wno-unused-result 
	)
ENDIF()

##############################################################
# link to the needed libs
SET (LIBS_LINUX
	clib_debugging
)

##############################################################
# define target directory
SET (TARGET_DIRECTORY
	${CMAKE_CURRENT_LIST_DIR}/../bin
)

##############################################################
# define binary name
SET (TARGET_LIB_NAME
	clib
)

##############################################################
# include CMake core
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CMakeCore.txt)

