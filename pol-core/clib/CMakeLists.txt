###############################################################################

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeEnvironment.txt)

MESSAGE("####################################")
MESSAGE("##### Building POL - CLIB")
MESSAGE("####################################")

###############################################################################
# define source files
SET (SOURCES  # sorted !
	../../lib/format/format.cc          	
	Header_Windows.h 								
	Debugging/ExceptionParser.cpp
	Debugging/LogSink.cpp
	Program/ProgramConfig.cpp
	Program/ProgramMain.cpp
	binaryfile.cpp 
	boostutils.cpp 
	cfgfile.cpp 
	cfgsect.cpp
	clib.h
	clibopt.h
	clib_MD5.cpp
	clib_utils.cpp 
	dirlist.cpp
	esignal.cpp
	fdump.cpp 
    filecont.cpp
	fileutil.cpp 
	iohelp.cpp
	kbhit.cpp
	logfacility.cpp
	mlog.cpp 
	opnew.cpp
	passert.cpp
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
IF (DEFINED ENABLE_BENCHMARK)
  SET (LIBS_LINUX
       ${LIBS_LINUX}
       benchmark
  )
ENDIF()

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

IF(NOT EXISTS "${BOOST_SOURCE_DIR}/boost")
ADD_DEPENDENCIES(clib boost)
ENDIF()
