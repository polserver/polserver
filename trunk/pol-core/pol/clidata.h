/*
History
=======


Notes
=======

*/

#ifndef CLIDATA_H
#define CLIDATA_H

bool groundheight( unsigned short x, unsigned short y, int* z );
char tileheight(unsigned short tilenum);
unsigned char tilelayer( unsigned short tilenum );
unsigned long tile_flags( unsigned short tilenum ); // POL flags
unsigned long tile_uoflags( unsigned short tilenum );
unsigned short tileweight( unsigned short tilenum );
string tile_desc( unsigned short tilenum );

unsigned long landtile_uoflags( unsigned short landtile );
unsigned long landtile_flags( unsigned short landtile );

#endif
