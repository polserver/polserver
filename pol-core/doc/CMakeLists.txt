set(lib_name doc)

message("* (${lib_name})")

#custom target to show it in IDEs
add_custom_target(${lib_name} SOURCES
  core-changes.txt
  packets.zip
  uoconvert.txt
  packethooks.txt
)

install(FILES
  core-changes.txt
  packets.zip
  uoconvert.txt
  packethooks.txt
  DESTINATION .
)


