#ifndef UOTOOL_CLIENTQUERIES_STANDHEIGHT_H
#define UOTOOL_CLIENTQUERIES_STANDHEIGHT_H

#include "../../plib/uconst.h"    // MOVEMODE
#include "../../plib/udatfile.h"  // StaticList

namespace Pol::Plib
{
class UoClientFiles;
}  // namespace Pol::Plib

namespace Pol::UoTool
{
// Walk/stand height simulation over raw client data (uotool). Uses the raw-map
// queries + tiledata flags, so the reader must have its map + tiledata loaded.
void standheight_read( const Pol::Plib::UoClientFiles& uof, Pol::Plib::MOVEMODE movemode,
                       Pol::Plib::StaticList& statics, unsigned short x, unsigned short y,
                       short oldz, bool* result, short* newz );
}  // namespace Pol::UoTool

#endif  // UOTOOL_CLIENTQUERIES_STANDHEIGHT_H
