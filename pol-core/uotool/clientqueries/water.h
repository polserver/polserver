#ifndef UOTOOL_CLIENTQUERIES_WATER_H
#define UOTOOL_CLIENTQUERIES_WATER_H

#include "../../clib/rawtypes.h"  // u16

namespace Pol::Plib
{
class UoClientFiles;
}  // namespace Pol::Plib

namespace Pol::UoTool
{
// Hardcoded water-graphic range predicate (uotool).
bool iswater( u16 objtype );

// Scans staidx/statics for water tiles, printing per-column progress (uotool
// `water` command). The reader must have open_uo_data_files() first.
void readwater( const Pol::Plib::UoClientFiles& uof );
}  // namespace Pol::UoTool

#endif  // UOTOOL_CLIENTQUERIES_WATER_H
