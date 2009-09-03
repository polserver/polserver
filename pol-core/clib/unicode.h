// History
//   2006/09/16 Shinigami: fixed Memory Overwrite Bug in convertArrayToUC

#ifndef CLIB_UNICODE_H
#define CLIB_UNICODE_H

#include <string>

#include "bscript/bobject.h"
#include "pol/utype.h"

//std::string toascii( const std::wstring& wstr );
//std::wstring tounicode( const std::string& str );

void unicode_copy_string( wchar_t* dst, size_t dstsize, const char* src );

bool convertArrayToUC(ObjArray*& in_text, u16* out_wtext,
					  unsigned textlen, bool ConvToBE = false, bool nullterm = true);
bool convertUCtoArray(const u16* in_wtext, ObjArray*& out_text,
					  unsigned textlen, bool ConvFromBE = false);

#endif
