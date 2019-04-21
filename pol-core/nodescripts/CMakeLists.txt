set(lib_name nodescripts)

message("* ${lib_name}")

add_custom_target(${lib_name} ALL SOURCES
  build-scripts/build.js
  build-scripts/em-parser.js
  build-scripts/install-deps.js
  build-scripts/wrap-generator.js
  build-scripts/package-lock.json
  templates/modules.hbs
  templates/objects.hbs
  # Generated files
  modules.js
  objects.js
)

SET_SOURCE_FILES_PROPERTIES(modules.js PROPERTIES GENERATED true)
SET_SOURCE_FILES_PROPERTIES(objects.js PROPERTIES GENERATED true)

add_custom_command(
  OUTPUT modules.js objects.js
  COMMAND nodebin install-deps.js ${CMAKE_COMMAND} 
  COMMAND nodebin build.js ${CMAKE_CURRENT_BINARY_DIR}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build-scripts
  DEPENDS nodebin
)

 install(FILES
   shard-scripts/main.js
   ${CMAKE_CURRENT_BINARY_DIR}/modules.js
   ${CMAKE_CURRENT_BINARY_DIR}/objects.js
   shard-scripts/wrapper.js
   shard-scripts/scriptloader.js
   DESTINATION .
   COMPONENT bin
 )

