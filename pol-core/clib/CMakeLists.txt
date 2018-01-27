set(lib_name clib)

if(windows)
  set (COND_SOURCES  # sorted !
		../../lib/StackWalker/StackWalker.cpp
		mdump.cpp
		msjexhnd.cpp
		strexcpt.cpp
	)
endif()

# define source files
add_library(${lib_name} STATIC
    # sorted !
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
	${COND_SOURCES}
)
set_compile_flags(${lib_name} 0)

use_benchmark(${lib_name})

target_link_libraries(${lib_name} PUBLIC
  $<${linux}:
	crypto
    pthread
  >
  $<${windows}:
	ws2_32
	Psapi
	DbgHelp
#	crypto
  >
)

