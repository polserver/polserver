cmake_minimum_required(VERSION 3.2)

if(NOT DEFINED ecompile_cfg)
  message(FATAL_ERROR "ecompile_cfg not defined")
endif()
if(NOT DEFINED testdir)
  message(FATAL_ERROR "testdir not defined")
endif()
if(NOT DEFINED modules)
  message(FATAL_ERROR "modules not defined")
endif()

configure_file(
    ${ecompile_cfg}
    ${testdir}/ecompile.cfg
  )
