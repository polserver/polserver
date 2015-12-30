###############################################################################

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeEnvironment.txt)

MESSAGE("####################################")
MESSAGE("##### Building BSCRIPT")
MESSAGE("####################################")

###############################################################################
# define source files
SET (SOURCES  # sorted !
	berror.cpp 
	blong.cpp 
	bstruct.cpp
	compctx.cpp 
	compiler.cpp
	compilercfg.cpp 
	dbl.cpp 
	dict.cpp 
	eprog.cpp
	eprog_read.cpp
	eprog2.cpp
	eprog3.cpp
	escrutil.cpp
	escript_config.cpp 
	escriptv.cpp 
	execmodl.cpp
	executor.cpp
	fmodule.cpp
	object.cpp 
	objstrm.cpp
	parser.cpp
	str.cpp 
	symcont.cpp
	tkn_strm.cpp 
	token.cpp
	tokens.cpp
	userfunc.cpp
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
	SET (FLAGS
		${FLAGS}
		-DINC_PASSERT=1 
	)
ENDIF()

##############################################################
# link to the needed libs
SET (LIBS_LINUX
    pthread    
)

##############################################################
# define target directory
SET (TARGET_DIRECTORY
	${CMAKE_CURRENT_LIST_DIR}/../bin
)

##############################################################
# define binary name
SET (TARGET_LIB_NAME
	bscript
)

##############################################################
# include CMake core
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CMakeCore.txt)

