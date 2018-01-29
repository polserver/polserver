set(lib_name plib)

add_library(${lib_name} STATIC
    # sorted !
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
)

set_compile_flags(${lib_name} 0)

target_link_libraries(${lib_name} PUBLIC
  clib
)

