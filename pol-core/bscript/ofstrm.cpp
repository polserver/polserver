/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#include <string.h>

#include "ofile.h"


#if 0

void OFile::printOn(ostream& os) const
{
   if (!fc) return;
   long fpos = -1;
   fc->tell(fpos);
   os << fc->filename_ << " offset " << fpos;
}

#endif
