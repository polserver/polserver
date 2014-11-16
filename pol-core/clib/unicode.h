/*
History
=======
2006/09/16 Shinigami: fixed Memory Overwrite Bug in convertArrayToUC

Notes
=======

*/

#ifndef CLIB_UNICODE_H
#define CLIB_UNICODE_H

#include <string>


#ifndef BSCRIPT_BOBJECT_H
#include "../bscript/bobject.h"
#endif

#include "../pol/utype.h"
namespace Pol {
  namespace Clib {
	//std::string toascii( const std::wstring& wstr );
	//std::wstring tounicode( const std::string& str );

	void unicode_copy_string( wchar_t* dst, size_t dstsize, const char* src );

	bool convertArrayToUC( Bscript::ObjArray*& in_text, u16* out_wtext,
						   size_t textlen, bool ConvToBE = false, bool nullterm = true );
	bool convertUCtoArray( const u16* in_wtext, Bscript::ObjArray*& out_text,
						   size_t textlen, bool ConvFromBE = false );
  }
}
#endif
