/*
History
=======

Notes
=======

*/

#ifndef CLIB_MAPUTIL_H
#define CLIB_MAPUTIL_H

#include "clib.h"
#include <cstring>

class ci_cmp_pred
{
public:
    bool operator()(const string& x1, const string& x2) const
    {
        return stricmp( x1.c_str(), x2.c_str() ) < 0;
    }
};

#endif
