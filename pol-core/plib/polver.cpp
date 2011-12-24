/*
History
=======
2005/08/19 Shinigami: Ready for VS.Net 2003
2006/06/09 Shinigami: Pol096 Final
2008/09/01 Shinigami: Pol097 Final
2009/04/14 Shinigami: Pol098 Final
2009/04/14 Shinigami: Pol099
2009/09/01 MuadDib:   VS 2008/2005 Proper Tags

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/progver.h"
#include "polver.h"

const char polverstr[] = "POL099-2011-05-02 Break Everything Even Rudder";
unsigned int polver = 99;

#ifdef PERGON
#define PERGON_TAG " - Pergon"
#else
#define PERGON_TAG ""
#endif

#ifdef MEMORYLEAK
#define MEMORYLEAK_TAG " - MemoryLeak"
#else
#define MEMORYLEAK_TAG ""
#endif

#ifdef ESCRIPT_PROFILE
#define PROFILE_TAG " - EscriptProfile"
#else
#define PROFILE_TAG ""
#endif

#if defined _WIN64 || defined _LP64
#define POL_64 " - 64bit"
#else
#define POL_64 ""
#endif

#ifdef POL_BUILDTAG
  const char polbuildtag[] = POL_BUILDTAG""PERGON_TAG""MEMORYLEAK_TAG""PROFILE_TAG""POL_64;
#elif defined(_MSC_VER) && (_MSC_VER <= 1200)
  const char polbuildtag[] = "VC6"PERGON_TAG""MEMORYLEAK_TAG""PROFILE_TAG""POL_64;
#elif defined(_MSC_VER) && (_MSC_VER <= 1300)
  const char polbuildtag[] = "VS.NET"PERGON_TAG""MEMORYLEAK_TAG""PROFILE_TAG""POL_64;
#elif defined(_MSC_VER) && (_MSC_VER <= 1310)
  const char polbuildtag[] = "VS.NET 2003"PERGON_TAG""MEMORYLEAK_TAG""PROFILE_TAG""POL_64;
#elif defined(_MSC_VER) && (_MSC_VER <= 1400)
  const char polbuildtag[] = "VS.NET 2005"PERGON_TAG""MEMORYLEAK_TAG""PROFILE_TAG""POL_64;
#elif defined(_MSC_VER) && (_MSC_VER <= 1500)
  const char polbuildtag[] = "VS.NET 2008"PERGON_TAG""MEMORYLEAK_TAG""PROFILE_TAG""POL_64;
#elif defined(_MSC_VER) && (_MSC_VER <= 1600)
  const char polbuildtag[] = "VS.NET 2010"PERGON_TAG""MEMORYLEAK_TAG""PROFILE_TAG""POL_64;
#endif
