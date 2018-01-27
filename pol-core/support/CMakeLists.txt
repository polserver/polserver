set(lib_name support)

install(FILES
  uoconvert.cfg
  pol.cfg
  DESTINATION .
)
install(FILES
  extobj.cfg
  boats.cfg
  animxlate.cfg
  DESTINATION config
)
install(FILES
  scripts/ecompile.cfg.example
  DESTINATION scripts
)
install(DIRECTORY scripts/
  DESTINATION scripts
  FILES_MATCHING PATTERN "*.em"
)
