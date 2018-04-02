/** @file
 *
 * @par History
 * - 2006/09/16 Shinigami: fixed Memory Overwrite Bug in convertArrayToUC
 */


#ifndef POL_UNICODE_H
#define POL_UNICODE_H

#include <string>

#include "../clib/rawtypes.h"

namespace Pol
{
namespace Bscript
{
class ObjArray;
}
namespace Core
{
bool convertArrayToUC( Bscript::ObjArray*& in_text, u16* out_wtext, size_t textlen,
                       bool ConvToBE = false, bool nullterm = true );
bool convertUCtoArray( const u16* in_wtext, Bscript::ObjArray*& out_text, size_t textlen,
                       bool ConvFromBE = false );
}
}
#endif
