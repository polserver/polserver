###############################################################################

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CMakeEnvironment.txt)

MESSAGE("####################################")
MESSAGE("##### Building UOTOOL")
MESSAGE("####################################")

###############################################################################
# define source files
SET (SOURCES  # sorted !
    ../pol/globals/multidefs.cpp        #TODO: ugly hack
    ../pol/multi/multidef.cpp           #TODO: ugly hack - what? same file name?
    ../pol/polfile1.cpp                 #TODO: w00t
    ../pol/uofile00.cpp                 #TODO: god damn, what an ugly hack
    ../pol/uofile01.cpp                 #TODO: and another one
    ../pol/uofile02.cpp                 #TODO: the story continues
    ../pol/uofile06.cpp                 #TODO: time for some headaches
    ../pol/uofile07.cpp                 #TODO: really bad headaches
    ../pol/uofile08.cpp                 #TODO: exploding head ... wtf
    uofile04.cpp                        #TODO: move it to some hidden place in the main pol folder and forget about header files
    uofile05.cpp                        #TODO: add 100 more uofile** files and use some cryptic function names, e.g. "do_something1" and "do_something42"
    UoToolMain.cpp                      #note to me: never ever again suggest to introduce CMake in a C style C++ project which is full of linking magic
)    

##############################################################
# include dirs
SET (INCLUDE_DIRS
    .
    ../
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
    plib
)
SET (LIBS_LINUX_STATIC_BIN_STATIC
    clib_static
    plib_static
)

##############################################################
# define target directory
SET (TARGET_DIRECTORY
    ${CMAKE_CURRENT_LIST_DIR}/../bin
)

##############################################################
# define binary name
SET (TARGET_PROGRAM_NAME
    uotool
)

##############################################################
# include CMake core
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CMakeCore.txt)
