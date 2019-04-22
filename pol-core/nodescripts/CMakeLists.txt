set(lib_name nodescripts)

message("* ${lib_name}")

add_custom_target(${lib_name} ALL
  COMMAND nodebin install-deps.js ${CMAKE_COMMAND} 
  COMMAND nodebin  build.js ${CMAKE_CURRENT_BINARY_DIR}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build-scripts
  DEPENDS nodebin
  SOURCES
  build-scripts/build.js
  build-scripts/em-parser.js
  build-scripts/install-deps.js
  build-scripts/wrap-generator.js
  build-scripts/package-lock.json
  templates/modules.hbs
  templates/objects.hbs
  # Generated files
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/modules.js ${CMAKE_CURRENT_BINARY_DIR}/objects.js
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

