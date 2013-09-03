/*
History
=======
2006/05/24 Shinigami: extended progverstr and buildtagstr upto 64 chars to hold Code names

Notes
=======

*/

#include "stl_inc.h"
#include "progver.h"

const char compiledate[] = __DATE__;
const char compiletime[] = __TIME__;

char progverstr[64];
char buildtagstr[64];
unsigned int progver;
