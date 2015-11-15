###############################################################################

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeEnvironment.txt)

MESSAGE("####################################")
MESSAGE("##### Building POL - CLIB")
MESSAGE("####################################")

###############################################################################
# define source files
SET (SOURCES  # sorted !
	../../lib/format/format.cc          	#TODO: cleanup for this hack
	Header_Windows.h 						#no auto-detect		
	Debugging/ExceptionParser.cpp
	Debugging/LogSink.cpp
	Program/ProgramConfig.cpp
	Program/ProgramMain.cpp
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
	#strexcpt.cpp #TODO: remove as default? 
	strset.cpp 
	strutil.cpp
	threadhelp.cpp
	timer.cpp
	tracebuf.cpp
	wallclock.cpp
	wnsckt.cpp
	xmain.cpp
)
IF(WINDOWS)
	SET (SOURCES  # sorted !
		${SOURCES}
		../../lib/StackWalker/StackWalker.cpp 	#TODO: cleanup for this hack
		mdump.cpp
		msjexhnd.cpp
		strexcpt.cpp
	)
ENDIF()

##############################################################
# include dirs
SET (INCLUDE_DIRS
	../..
	../
	./
	./Debugging
	./Program
)

##############################################################
# set directory for internal libs
SET (LIB_DIRS
	${LIB_DIRS}
	${CMAKE_CURRENT_LIST_DIR}/../bin
)

##############################################################
# Linux specific dependencies
SET (LIBS_LINUX
	crypto
    pthread    
)

##############################################################
# Windows specific dependencies
SET (LIBS_WINDOWS
	ws2_32
	Psapi
	DbgHelp
#	crypto
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

