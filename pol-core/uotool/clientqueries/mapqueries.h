#ifndef UOTOOL_CLIENTQUERIES_MAPQUERIES_H
#define UOTOOL_CLIENTQUERIES_MAPQUERIES_H

namespace Pol::Plib
{
class UoClientFiles;
struct USTRUCT_MAPINFO;
}  // namespace Pol::Plib

namespace Pol::UoTool
{
// Raw-map queries with neighbour z-averaging over a loaded UoClientFiles raw map.
// uotool-only diagnostics; uoconvert copies the whole plane via
// UoClientFiles::rawmap_extract_planes instead. The reader must be rawmapfullread()
// first -- these assert it, they do not load.
void getmapinfo( const Pol::Plib::UoClientFiles& uof, unsigned short x, unsigned short y,
                 short* z, Pol::Plib::USTRUCT_MAPINFO* mi );
void safe_getmapinfo( const Pol::Plib::UoClientFiles& uof, unsigned short x, unsigned short y,
                      short* z, Pol::Plib::USTRUCT_MAPINFO* mi );
bool groundheight_read( const Pol::Plib::UoClientFiles& uof, unsigned short x, unsigned short y,
                        short* z );
}  // namespace Pol::UoTool

#endif  // UOTOOL_CLIENTQUERIES_MAPQUERIES_H
