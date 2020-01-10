# Variable definition MODMAP_<modname>
# Maps a .em filename to a tuple of:
# - C++ class name
# - module name
set(MODMAP_attributes AttributeExecutorModule attributemod)
set(MODMAP_basic BasicExecutorModule basicmod)
set(MODMAP_basicio BasicIoExecutorModule basiciomod)
set(MODMAP_boat UBoatExecutorModule boatmod)
set(MODMAP_cfgfile ConfigFileExecutorModule cfgmod)
set(MODMAP_cliloc ClilocExecutorModule clmod)
set(MODMAP_datafile DataFileExecutorModule datastore)
set(MODMAP_file FileAccessExecutorModule filemod)
set(MODMAP_guilds GuildExecutorModule guildmod)
set(MODMAP_http HttpExecutorModule httpmod)
set(MODMAP_math MathExecutorModule mathmod)
set(MODMAP_npc NPCExecutorModule npcmod)
set(MODMAP_os OSExecutorModule osmod)
set(MODMAP_party PartyExecutorModule partymod)
set(MODMAP_polsys PolSystemExecutorModule polsystemmod)
set(MODMAP_sql SQLExecutorModule sqlmod)
set(MODMAP_storage StorageExecutorModule storagemod)
set(MODMAP_unicode UnicodeExecutorModule unimod)
set(MODMAP_uo UOExecutorModule uomod)
set(MODMAP_util UtilExecutorModule utilmod)
set(MODMAP_vitals VitalExecutorModule vitalmod)

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

  foreach(em ${ems})
    get_filename_component(em_name ${em} NAME_WE)
    set(em "${EM_FOLDER}/${em_name}.em")
    # Get module settings from map
    LIST(GET MODMAP_${em_name} 0 MODCLASS)
    LIST(GET MODMAP_${em_name} 1 MODHEADER)

    set(HEADER "#ifndef ${MODCLASS}_EM_h\n#define ${MODCLASS}_EM_h\n")
    set(HEADER "${HEADER}namespace Pol\n{\nnamespace Bscript\n{\nusing namespace Module\;\n\n")
  
    # message("Creating function table definition for ${em} => ${em_name} => ${MODCLASS}")
    set(MODDEF "template <>\nconst char* const TmplExecutorModule<${MODCLASS}, Bscript::ExecutorModule>::modname = \"${em_name}\"\;\n\ntemplate <>\nTmplExecutorModule<${MODCLASS}, Bscript::ExecutorModule>::FunctionTable\n  TmplExecutorModule<${MODCLASS}, Bscript::ExecutorModule>::function_table = {\n")
    FILE(READ ${em} contents)
    STRING(REGEX REPLACE ";" "\\\\;" contents "${contents}")
    read_def(${MODCLASS} ${contents} FUNCTBL)
    set(MODDEF "${MODDEF}${FUNCTBL}}\;\n\n")
    
    set(FOOTER "} // namespace Bscript\n} // namespace Pol\n#endif")
    set(OUT_MOD "${HEADER}\n${MODDEF}${FOOTER}")

    file(WRITE "${OUT_FOLDER}/${em_name}.h" ${OUT_MOD})
  endforeach()
endfunction()

createfunctable()
