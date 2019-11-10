# Variable definition MODMAP_<modname>
# Maps a .em filename to a tuple of:
# - C++ class name
# - module name
# - For runecl: TRUE if usable, FALSE otherwise
set(MODMAP_attributes AttributeExecutorModule attributemod FALSE)
set(MODMAP_basic BasicExecutorModule basicmod TRUE)
set(MODMAP_basicio BasicIoExecutorModule basiciomod TRUE)
set(MODMAP_boat UBoatExecutorModule boatmod FALSE)
set(MODMAP_cfgfile ConfigFileExecutorModule cfgmod TRUE)
set(MODMAP_cliloc ClilocExecutorModule clmod FALSE)
set(MODMAP_datafile DataFileExecutorModule datastore TRUE)
set(MODMAP_file FileAccessExecutorModule filemod TRUE)
set(MODMAP_guilds GuildExecutorModule guildmod FALSE)
set(MODMAP_http HttpExecutorModule httpmod FALSE)
set(MODMAP_math MathExecutorModule mathmod TRUE)
set(MODMAP_npc NPCExecutorModule npcmod FALSE)
set(MODMAP_os OSExecutorModule osmod FALSE)
set(MODMAP_party PartyExecutorModule partymod FALSE)
set(MODMAP_polsys PolSystemExecutorModule polsystemmod FALSE)
set(MODMAP_sql SQLExecutorModule sqlmod FALSE)
set(MODMAP_storage StorageExecutorModule storagemod FALSE)
set(MODMAP_unicode UnicodeExecutorModule unimod FALSE)
set(MODMAP_uo UOExecutorModule uomod FALSE)
set(MODMAP_util UtilExecutorModule utilmod TRUE)
set(MODMAP_vitals VitalExecutorModule vitalmod FALSE)

if(NOT EM_FOLDER)
  message(FATAL_ERROR "No EM_FOLDER set")
endif()
if(NOT OUT_FOLDER)
  message(FATAL_ERROR "No OUT_FOLDER set")
endif()

# Parses the `CONTENT` of an .em file to create the function table lines:
#  { "FuncName", &MODCLASS::mf_FuncName, funcArgCount }, // func args from .em
function (read_def MODCLASS CONTENT OUT_APPEND_TO)

  # 
  set(funcdefs "")
  while(1)

    # If empty string, finished processing. Break.
    string(LENGTH "${CONTENT}" out)
    if (${out} EQUAL 0)
      break()
    endif()

    # Trim whitespaces
    string(STRIP "${CONTENT}" CONTENT)

    # Skip line comments
    string(FIND "${CONTENT}" "//" out)
    if(${out} EQUAL 0)
      string(FIND "${CONTENT}" "\n" out)
      if(${out} GREATER_EQUAL 0)
        string(SUBSTRING "${CONTENT}" ${out} -1 CONTENT)
        continue()
      endif()
    endif()

    # Skip `const` declarations
    string(FIND "${CONTENT}" "const" out)
    if(${out} EQUAL 0)
      string(FIND "${CONTENT}" "\n" out)
      if(${out} GREATER_EQUAL 0)    
        string(SUBSTRING "${CONTENT}" ${out} -1 CONTENT)
      endif()
      continue()
    endif()

    # Check for module function declaration. If none, break.
    string(REGEX MATCH "^([a-zA-Z_0-9]+)[\\( ]+([^;]*)[ \\)]+;" out "${CONTENT}")
    string(LENGTH "${out}" out)
    if (${out} EQUAL 0)
      break()
    endif()

    set(funcname "${CMAKE_MATCH_1}")
    #  message("found :")
    #  message(${funcname})
    string(APPEND funcdefs  "    { \"${funcname}\", &${MODCLASS}::mf_${funcname}, ")
    if (CMAKE_MATCH_2)
      set(paramstr "${CMAKE_MATCH_2}")
      #  message(${paramstr})
      string(REGEX MATCHALL "," params "${paramstr}")
      list(LENGTH params n)
      math(EXPR n ${n}+1)
      #  message("params: ${n}")
      string(APPEND funcdefs "${n} }, // ${paramstr}\n")
    else()
      #  message("params: 0")
      string(APPEND funcdefs "0 },\n")
    endif()

    string(SUBSTRING "${CONTENT}" ${out} -1 CONTENT)
  endwhile()
  set(${OUT_APPEND_TO} "${funcdefs}" PARENT_SCOPE)
endfunction()

function(createfunctable)
  file(GLOB ems  "${EM_FOLDER}/*.em")

  # Variables:
  # - OUT_POL: Accumulation of C++ code for pol, written at end of function
  # - OUT_RUNECL: Accumulation of C++ code for runecl, written at end of function
  # - INCLUDE_POL: '#include' lines for pol
  # - INCLUDE_RUNECL: '#include' lines for runecl
  # - LINE_OUT: temporary C++ to append to OUT_POL/OUT_ECL
  set(INCLUDE_POL "")
  set(INCLUDE_RUNECL "")

  set(LINE_OUT "namespace Pol\n{\nnamespace Bscript\n{\nusing namespace Module\;\n\n")
  set(OUT_RUNECL "${LINE_OUT}")
  set(OUT_POL "${LINE_OUT}")

  foreach(em ${ems})
    get_filename_component(em_name ${em} NAME_WE)
    set(em "${EM_FOLDER}/${em_name}.em")
    # Get module settings from map
    LIST(GET MODMAP_${em_name} 0 MODCLASS)
    LIST(GET MODMAP_${em_name} 1 MODHEADER)
    LIST(GET MODMAP_${em_name} 2 IS_RUNECL_USABLE)

    # message("Creating function table definition for ${em} => ${em_name} => ${MODCLASS}")
    set(INCLUDE_POL "${INCLUDE_POL}#include \"module/${MODHEADER}.h\"\n")
    set(MODDEF "template <>\nconst char* TmplExecutorModule<${MODCLASS}>::modname = \"${em_name}\"\;\n\ntemplate <>\nTmplExecutorModule<${MODCLASS}>::FunctionTable\n  TmplExecutorModule<${MODCLASS}>::function_table = {\n")
    FILE(READ ${em} contents)
    STRING(REGEX REPLACE ";" "\\\\;" contents "${contents}")
    read_def(${MODCLASS} ${contents} FUNCTBL)
    set(MODDEF "${MODDEF}${FUNCTBL}}\;\n\n")
    
    set(OUT_POL "${OUT_POL}${MODDEF}")
    if(IS_RUNECL_USABLE)
      set(INCLUDE_RUNECL "${INCLUDE_RUNECL}#include \"../pol/module/${MODHEADER}.h\"\n")
      set(OUT_RUNECL "${OUT_RUNECL}${MODDEF}")
    endif()

  endforeach()
  set(LINE_OUT "} // namespace Bscript\n} // namespace Pol\n")
  set(OUT_RUNECL "${INCLUDE_RUNECL}\n${OUT_RUNECL}${LINE_OUT}")
  set(OUT_POL "${INCLUDE_POL}\n${OUT_POL}${LINE_OUT}")

  set(OUT_FILE_POL "${OUT_FOLDER}/modtbl-pol.cpp")
  set(OUT_FILE_RUNECL "${OUT_FOLDER}/modtbl-runecl.cpp")

  file(WRITE ${OUT_FILE_POL} ${OUT_POL})
  file(WRITE ${OUT_FILE_RUNECL} ${OUT_RUNECL})
endfunction()

createfunctable()
