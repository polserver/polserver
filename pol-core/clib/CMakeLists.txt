###############################################################################

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeEnvironment.txt)

MESSAGE("####################################")
MESSAGE("##### Building POL - CLIB")
MESSAGE("####################################")

###############################################################################
# define source files
SET (SOURCES  # sorted !
	../../lib/format/format.cc
	Debugging/ConfigEnvironment.cpp
	Debugging/ExceptionParser.cpp			
	Debugging/LogSink.cpp
	binaryfile.cpp 
	boostutils.cpp 
	cfgfile.cpp 
	cfgsect.cpp
	clib_utils.cpp 
	cmdargs.cpp 
	dirlist.cpp
	esignal.cpp
	fdump.cpp 
    filecont.cpp
	fileutil.cpp 
	iohelp.cpp
	kbhit.cpp
	logfacility.cpp
	MD5.cpp
	mlog.cpp 
	opnew.cpp
	passert.cpp
	pkthelper.cpp
	random.cpp 
	sckutil.cpp 
	socketsvc.cpp 
	stracpy.cpp
	streamsaver.cpp
	strexcpt.cpp 
	strset.cpp 
	strutil.cpp
	threadhelp.cpp
	timer.cpp
	tracebuf.cpp
	wallclock.cpp
	wnsckt.cpp
	xmain.cpp
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
	crypto
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

