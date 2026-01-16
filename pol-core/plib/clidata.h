/** @file
 *
 * @par History
 */


#ifndef CLIDATA_H
#define CLIDATA_H

#include <string>


namespace Pol::Plib
{
bool groundheight_read( unsigned short x, unsigned short y, short* z );  // uotool version
char tileheight( unsigned short tilenum );
char tileheight_read( unsigned short tilenum );  // uotool version
unsigned char tilelayer( unsigned short tilenum );
unsigned char tilelayer_read( unsigned short tilenum );  // uotool version
unsigned int tile_flags( unsigned short tilenum );       // POL flags
unsigned int tile_uoflags( unsigned short tilenum );
unsigned int tile_uoflags_read( unsigned short tilenum );  // uotool version
unsigned short tileweight( unsigned short tilenum );
unsigned short tileweight_read( unsigned short tilenum );  // uotool version
std::string tile_desc( unsigned short tilenum );

unsigned int landtile_uoflags_read( unsigned short landtile );  // uotool version

const unsigned LANDTILE_COUNT = 0x4000;
struct LandTile
{
  unsigned int uoflags;
  unsigned int flags;
};
}  // namespace Pol::Plib

#endif
