if (STRING_ENCODING_TYPE STREQUAL "ICONV")
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DKS_STR_ENCODING_ICONV)
elseif (STRING_ENCODING_TYPE STREQUAL "WIN32API")
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DKS_STR_ENCODING_WIN32API)
elseif (STRING_ENCODING_TYPE STREQUAL "NONE")
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DKS_STR_ENCODING_NONE)
else()
    # User action requested
endif()

# Maximum warnings emission, treat all warnings as errors
if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
