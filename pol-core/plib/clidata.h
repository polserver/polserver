/** @file
 *
 * @par History
 */


#ifndef CLIDATA_H
#define CLIDATA_H

#include <string>


namespace Pol::Plib
{
// The *_read (client-mul backed) family used to live here as free functions;
// it is now UoClientFiles methods (uoclientfiles.h) -- used only by
// uoconvert/uotool. What remains here is the tiles.cfg/pol-file backed family
// (polfile2.cpp), used by the server.
char tileheight( unsigned short tilenum );
unsigned char tilelayer( unsigned short tilenum );
unsigned int tile_flags( unsigned short tilenum );  // POL flags
unsigned int tile_uoflags( unsigned short tilenum );
unsigned short tileweight( unsigned short tilenum );
std::string tile_desc( unsigned short tilenum );

const unsigned LANDTILE_COUNT = 0x4000;
// Landtile ids are 14-bit; anything above this is not a valid landtile.
constexpr unsigned short MAX_LANDTILE_ID = LANDTILE_COUNT - 1;
struct LandTile
{
  unsigned int uoflags;
  unsigned int flags;
};
}  // namespace Pol::Plib

#endif
