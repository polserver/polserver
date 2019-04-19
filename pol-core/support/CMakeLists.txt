set(lib_name support)

message("* (${lib_name})")

#custom target to show it in IDEs
add_custom_target(${lib_name} SOURCES
  animxlate.cfg
  boats.cfg
  extobj.cfg
  pol.cfg.example
  scripts/ecompile.cfg.example
  scripts/attributes.em
  scripts/basic.em
  scripts/basicio.em
  scripts/boat.em
  scripts/cfgfile.em
  scripts/cliloc.em
  scripts/datafile.em
  scripts/file.em
  scripts/guilds.em
  scripts/http.em
  scripts/math.em
  scripts/npc.em
  scripts/os.em
  scripts/party.em
  scripts/polsys.em
  scripts/sql.em
  scripts/storage.em
  scripts/unicode.em
  scripts/uo.em
  scripts/util.em
  scripts/vitals.em
  uoconvert.cfg
)

install(FILES
  pol.cfg.example
  uoconvert.cfg
  DESTINATION .
  COMPONENT bin
)
install(FILES
  extobj.cfg
  boats.cfg
  animxlate.cfg
  DESTINATION config
  COMPONENT bin
)
install(FILES
  scripts/ecompile.cfg.example
  DESTINATION scripts
  COMPONENT bin
)
install(FILES
  scripts/attributes.em
  scripts/basic.em
  scripts/basicio.em
  scripts/boat.em
  scripts/cfgfile.em
  scripts/cliloc.em
  scripts/datafile.em
  scripts/file.em
  scripts/guilds.em
  scripts/http.em
  scripts/math.em
  scripts/npc.em
  scripts/os.em
  scripts/party.em
  scripts/polsys.em
  scripts/sql.em
  scripts/storage.em
  scripts/unicode.em
  scripts/uo.em
  scripts/util.em
  scripts/vitals.em
  DESTINATION scripts/modules
  COMPONENT bin
)

set_target_properties (${lib_name} PROPERTIES FOLDER Misc)
