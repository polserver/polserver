set(lib_name bscript)

add_library(${lib_name} STATIC
    # sorted !
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

set_compile_flags(${lib_name} 0)

target_link_libraries(${lib_name} PUBLIC
  clib
  pthread
)

