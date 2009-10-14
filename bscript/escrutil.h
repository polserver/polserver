/*
History
=======

Notes
=======

*/

#ifndef BSCRIPT_ESCRUTIL_H
#define BSCRIPT_ESCRUTIL_H

#include <string>
class BObjectImp;

BObjectImp* bobject_from_string( const std::string& str, int radix = 0 );
std::string normalize_ecl_filename( const std::string& filename );

#endif
