#ifndef UOTOOL_CLIENTQUERIES_STATICSQUERIES_H
#define UOTOOL_CLIENTQUERIES_STATICSQUERIES_H

#include "../../plib/udatfile.h"  // StaticList

namespace Pol::Plib
{
class UoClientFiles;
}  // namespace Pol::Plib

namespace Pol::UoTool
{
// Gates the optional per-record debug prints in the statics/walk-sim queries
// (only compiled in under ENABLE_POLTEST_OUTPUT).
extern bool static_debug_on;

// Per-tile statics queries over the reader's block cache (uotool diagnostics;
// uoconvert uses UoClientFiles::readstatics_block instead). The reader must be
// rawstaticfullread() first -- getstaticblock asserts it.
void readstatics( const Pol::Plib::UoClientFiles& uof, Pol::Plib::StaticList& vec, unsigned short x,
                  unsigned short y );
void readstatics( const Pol::Plib::UoClientFiles& uof, Pol::Plib::StaticList& vec, unsigned short x,
                  unsigned short y, unsigned int flags );
void readallstatics( const Pol::Plib::UoClientFiles& uof, Pol::Plib::StaticList& vec,
                     unsigned short x, unsigned short y );

// Scans staidx for the largest block (diagnostic print).
void staticsmax( const Pol::Plib::UoClientFiles& uof );
}  // namespace Pol::UoTool

#endif  // UOTOOL_CLIENTQUERIES_STATICSQUERIES_H
