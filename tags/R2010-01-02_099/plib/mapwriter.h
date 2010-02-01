/*
History
=======
2006/10/28 Shinigami: GCC 4.1.1 fix - extra qualification in class removed

Notes
=======

*/

#ifndef PLIB_MAPWRITER_H
#define PLIB_MAPWRITER_H

struct SOLIDS_ELEM;
struct SOLIDX2_ELEM;
struct MAPTILE_CELL;

#include "maptile.h"

class MapWriter
{
public:
    MapWriter();
    ~MapWriter();

    void SetMapCell( unsigned short x, unsigned short y, MAPCELL cell );
    void SetMapTile( unsigned short x, unsigned short y, MAPTILE_CELL cell );

    void Flush();

    void CreateNewFiles(const string& realm_name, unsigned short width, unsigned short height);
    void OpenExistingFiles(const string& realm_name);
    void WriteConfigFile();

    unsigned long NextSolidIndex();
    unsigned long NextSolidOffset();

    unsigned long NextSolidx2Offset();

    void AppendSolid( const SOLIDS_ELEM& solid );
    void AppendSolidx2Elem( const SOLIDX2_ELEM& elem );
    void SetSolidx2Offset( unsigned short x_base, unsigned short y_base, unsigned long offset );

    unsigned width() const { return _width; }
    unsigned height() const { return _height; }
private:
    std::fstream::pos_type total_size();
    
    unsigned long total_solid_blocks();
    unsigned long total_blocks();
    unsigned long total_maptile_blocks();

    void FlushBaseFile();
    void FlushMapTileFile();

private:
    string _realm_name;
    unsigned short _width;
    unsigned short _height;
    fstream _ofs_base;
    long _cur_mapblock_index;
    MAPBLOCK _block;

    fstream _ofs_solidx1;
    fstream _ofs_solidx2;
    fstream _ofs_solids;

    fstream _ofs_maptile;
    long _cur_maptile_index;
    MAPTILE_BLOCK _maptile_block;
};

#endif
