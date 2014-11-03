/*
History
=======
2006/05/24 Shinigami: extended progverstr and buildtagstr upto 64 chars to hold Code names

Notes
=======

*/
#include "progver.h"
#include "../svn_version.h"

namespace Pol {
	const char compiledate[] = __DATE__;
#ifdef _SVNVER
	const char compiletime[] = __TIME__ " (Rev. " _SVNVER ")";
#else
    const char compiletime[] = __TIME__;
#endif
	char progverstr[64];
	char buildtagstr[64];
	unsigned int progver;
}