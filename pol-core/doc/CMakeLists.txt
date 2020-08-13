set(lib_name doc)

message("* (${lib_name})")

#custom target to show it in IDEs
add_custom_target(${lib_name} SOURCES
  breaking-changes.txt
  core-changes.txt
  packets.zip
  uoconvert.txt
  packethooks.txt
)

install(FILES
  breaking-changes.txt
  core-changes.txt
  packets.zip
  uoconvert.txt
  packethooks.txt
  DESTINATION .
  COMPONENT bin
)

set_target_properties (${lib_name} PROPERTIES FOLDER Misc)


