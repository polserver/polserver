/*
History
=======


Notes
=======

*/

#ifndef CLIDATA_H
#define CLIDATA_H

bool groundheight( unsigned short x, unsigned short y, short* z );
char tileheight(unsigned short tilenum);
unsigned char tilelayer( unsigned short tilenum );
unsigned int tile_flags( unsigned short tilenum ); // POL flags
unsigned int tile_uoflags( unsigned short tilenum );
unsigned short tileweight( unsigned short tilenum );
std::string tile_desc( unsigned short tilenum );

unsigned int landtile_uoflags( unsigned short landtile );
unsigned int landtile_flags( unsigned short landtile );

#endif
