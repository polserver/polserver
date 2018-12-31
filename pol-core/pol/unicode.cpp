/** @file
 *
 * @par History
 * - 2006/09/16 Shinigami: fixed Memory Overwrite Bug in convertArrayToUC
 */


#include "unicode.h"

#include <stddef.h>

#include "../bscript/bobject.h"
#include "../clib/clib_endian.h"

namespace Pol
{
namespace Core
{

bool convertUCtoArray( const u16* in_wtext, Bscript::ObjArray*& out_text, size_t textlen,
                       bool ConvFromBE /*false*/ )
{
  u16 value;
  out_text = new Bscript::ObjArray();
  for ( size_t i = 0; i < textlen; i++ )
  {
    value = ConvFromBE ? cfBEu16( in_wtext[i] ) : in_wtext[i];
    // Jump out on a nullptr (EOS) value (should stop exploits, too?)
    if ( value == 0L )
      return true;
    out_text->addElement( new Bscript::BLong( value ) );
  }
  return true;
}
}
}
