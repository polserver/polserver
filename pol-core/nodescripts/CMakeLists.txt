set(lib_name nodescripts)

message("* ${lib_name}")

add_custom_target(${lib_name} SOURCES
  main.js
  modules.js
  wrapper.js
  scriptloader.js
)

SET_SOURCE_FILES_PROPERTIES(modules.js PROPERTIES GENERATED true)

add_custom_command(
  OUTPUT modules.js
  COMMAND nodebin "${CMAKE_CURRENT_SOURCE_DIR}/parser.js"
  DEPENDS nodebin
)

 install(FILES
   main.js
   ${CMAKE_CURRENT_BINARY_DIR}/modules.js
   wrapper.js
   scriptloader.js
   DESTINATION .
   COMPONENT bin
 )

