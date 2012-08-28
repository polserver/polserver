/* We do not use auto link feature when:
 *  - user asked not to use it (_STLP_DONT_USE_AUTO_LINK)
 *  - STLport is used only as a STL library (_STLP_NO_IOSTREAMS || _STLP_USE_NO_IOSTREAMS)
 *  - we are building a C translation unit, STLport is a C++ Standard library implementation
 */
#if !defined (__BUILDING_STLPORT) && !defined (_STLP_DONT_USE_AUTO_LINK) && \
    !defined (_STLP_NO_IOSTREAMS) && !defined (_STLP_USE_NO_IOSTREAMS) && \
    defined (__cplusplus)

#  define _STLP_STRINGIZE(X) _STLP_STRINGIZE_AUX(X)
#  define _STLP_STRINGIZE_AUX(X) #X

#  if defined (_STLP_DEBUG)
#    define _STLP_LIB_OPTIM_MODE "stld"
#  elif defined (_DEBUG)
#    define _STLP_LIB_OPTIM_MODE "d"
#  else
#    define _STLP_LIB_OPTIM_MODE ""
#  endif

#  if defined (_STLP_LIB_NAME_MOTIF)
#    define _STLP_LIB_MOTIF "_"_STLP_LIB_NAME_MOTIF
#  else
#    define _STLP_LIB_MOTIF ""
#  endif

#  if defined (_STLP_USE_DYNAMIC_LIB)
#    if defined (_STLP_USING_CROSS_NATIVE_RUNTIME_LIB)
#      define _STLP_LIB_TYPE "_x"
#    else
#      define _STLP_LIB_TYPE ""
#    endif
#  else
#    if defined (_STLP_USING_CROSS_NATIVE_RUNTIME_LIB)
#      define _STLP_LIB_TYPE "_statix"
#    else
#      define _STLP_LIB_TYPE "_static"
#    endif
#  endif

// LFH - always include version information

#  if defined (_STLP_USE_DYNAMIC_LIB)
#    define _STLP_VERSION_STR "."_STLP_STRINGIZE(_STLPORT_MAJOR)"."_STLP_STRINGIZE(_STLPORT_MINOR)
#  else
#    define _STLP_VERSION_STR ""
#  endif

// Start LFH
// Put back the useful, working code from pre 5.0.0 that allows coexistance of multiple VC version builds of
// STLPort, and add a case for VC8 and VC9. Also add in _STLP_LIB_ARCHNAME so that x64 builds can co-exist.

#ifdef _M_X64
#define _STLP_LIB_ARCHNAME "_x64"
#else
#define _STLP_LIB_ARCHNAME ""
#endif

# ifdef __ICL
#  define _STLP_LIB_BASENAME "stlport_icl"_STLP_LIB_ARCHNAME
# else
# if (_MSC_VER >= 1700) 
#   define _STLP_LIB_BASENAME "stlport_vc11"_STLP_LIB_ARCHNAME
# elif (_MSC_VER >= 1600) 
#   define _STLP_LIB_BASENAME "stlport_vc10"_STLP_LIB_ARCHNAME
# elif (_MSC_VER >= 1500) 
#   define _STLP_LIB_BASENAME "stlport_vc9"_STLP_LIB_ARCHNAME
# elif (_MSC_VER >= 1400) 
#   define _STLP_LIB_BASENAME "stlport_vc8"_STLP_LIB_ARCHNAME
# elif (_MSC_VER >= 1310) 
#   define _STLP_LIB_BASENAME "stlport_vc71"_STLP_LIB_ARCHNAME
# elif (_MSC_VER >= 1300) 
#   define _STLP_LIB_BASENAME "stlport_vc70"_STLP_LIB_ARCHNAME
# elif (_MSC_VER >= 1200)
//#   ifdef _UNICODE
//#    define _STLP_LIB_BASENAME "stlport_vc6_unicode"_STLP_LIB_ARCHNAME
//#   else
#    define _STLP_LIB_BASENAME "stlport_vc6"_STLP_LIB_ARCHNAME
//#   endif
#  elif (_MSC_VER >= 1100)
//#   ifdef _UNICODE
//#    define _STLP_LIB_BASENAME "stlport_vc5_unicode"_STLP_LIB_ARCHNAME
//#   else
#    define _STLP_LIB_BASENAME "stlport_vc5"_STLP_LIB_ARCHNAME
//#   endif
#  endif /* (_MSC_VER >= 1200) */
# endif /* __ICL */



// end LFH


//#  define _STLP_STLPORT_LIB _STLP_LIB_BASENAME""_STLP_LIB_OPTIM_MODE""_STLP_LIB_TYPE"."_STLP_VERSION_STR".lib"
//#  define _STLP_STLPORT_LIB "stlport"_STLP_LIB_OPTIM_MODE""_STLP_LIB_TYPE""_STLP_LIB_MOTIF"."_STLP_VERSION_STR".lib"
#  define _STLP_STLPORT_LIB _STLP_LIB_BASENAME""_STLP_LIB_OPTIM_MODE""_STLP_LIB_TYPE""_STLP_VERSION_STR".lib"

#  if defined (_STLP_VERBOSE)
#    pragma message ("STLport: Auto linking to "_STLP_STLPORT_LIB)
#  endif
#  pragma comment (lib, _STLP_STLPORT_LIB)

#  undef _STLP_STLPORT_LIB
#  undef _STLP_LIB_OPTIM_MODE
#  undef _STLP_LIB_TYPE
#  undef _STLP_STRINGIZE_AUX
#  undef _STLP_STRINGIZE

#endif /* _STLP_DONT_USE_AUTO_LINK */

