/*
History
=======
2006/09/16 Shinigami: fixed Memory Overwrite Bug in convertArrayToUC

Notes
=======

*/

#include "stl_inc.h"

#include "endian.h"
#include "unicode.h"
namespace Pol {
  namespace Clib {
	// Warning: Make sure that your buffer if large enough! Don't forget to add space for null terminator if requested.
	bool convertArrayToUC( Bscript::ObjArray*& in_text, u16* out_wtext,
						   size_t textlen, bool ConvToBE /*false*/, bool nullterm /*true*/ )
	{
	  u16 value;
	  size_t pos = 0;
	  for( size_t i = 0; i < textlen; i++ )
	  {
		Bscript::BObject* bo = in_text->ref_arr[i].get( );
		if( bo == NULL )
		  continue;
		if ( !bo->isa( Bscript::BObjectImp::OTLong ) )
		  return false;
		Bscript::BObjectImp *imp = bo->impptr( );
		Bscript::BLong* blong = static_cast<Bscript::BLong*>( imp );
		value = (u16)( blong->value() & 0xFFFF );
		out_wtext[pos++] = ConvToBE ? ctBEu16( value ) : value;
		// Jump out on a NULL (EOS) value (should stop exploits, too?)
		if( value == 0L )
		  return true;
	  }
	  if( nullterm )
		out_wtext[pos] = (u16)0L;
	  return true;
	}

	bool convertUCtoArray( const u16* in_wtext, Bscript::ObjArray*& out_text,
						   size_t textlen, bool ConvFromBE /*false*/ )
	{
	  u16 value;
	  out_text = new Bscript::ObjArray( );
	  for( size_t i = 0; i < textlen; i++ )
	  {
		value = ConvFromBE ? cfBEu16( in_wtext[i] ) : in_wtext[i];
		// Jump out on a NULL (EOS) value (should stop exploits, too?)
		if( value == 0L )
		  return true;
		out_text->addElement( new Bscript::BLong( value ) );
	  }
	  return true;
	}
  }
}