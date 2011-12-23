/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/iohelp.h"

#include "mapblock.h"
#include "mapserver.h"
#include "mapwriter.h"
#include "mapsolid.h"
#include "maptile.h"

#include "../pol/uofile.h"


extern unsigned int num_map_patches;
extern unsigned int num_static_patches;

MapWriter::MapWriter() :
    _realm_name( "" ),
    _width( 0 ),
    _height( 0 ),
    _ofs_base(),
    _cur_mapblock_index(-1),
    _ofs_solidx1(),
    _ofs_solidx2(),
    _ofs_solids(),
    _ofs_maptile(),
    _cur_maptile_index(-1)
{
    _ofs_base.exceptions( std::ios_base::failbit | std::ios_base::badbit );
    _ofs_solidx1.exceptions( std::ios_base::failbit | std::ios_base::badbit );
    _ofs_solidx2.exceptions( std::ios_base::failbit | std::ios_base::badbit );
    _ofs_solids.exceptions( std::ios_base::failbit | std::ios_base::badbit );
    _ofs_maptile.exceptions( std::ios_base::failbit | std::ios_base::badbit );
}

void MapWriter::WriteConfigFile()
{
    string filename = "realm/" + _realm_name + "/realm.cfg";
    ofstream ofs_cfg;
    ofs_cfg.exceptions( std::ios_base::failbit | std::ios_base::badbit );

    open_file( ofs_cfg, filename, ios::trunc|ios::out );

    ofs_cfg << "Realm " << _realm_name << endl;
    ofs_cfg << "{" << endl;
    ofs_cfg << "    width " << _width << endl;
    ofs_cfg << "    height " << _height << endl;
    ofs_cfg << "    mapserver memory" << endl;
    ofs_cfg << "    uomapid " << uo_mapid << endl;
    ofs_cfg << "    uodif " << uo_usedif << endl;
	ofs_cfg << "    num_static_patches " << num_static_patches << endl;
	ofs_cfg << "    num_map_patches " << num_map_patches << endl;
	ofs_cfg << "    season 1"<< endl;
    ofs_cfg << "}" << endl;
}

void MapWriter::CreateNewFiles(const string& realm_name, unsigned short width, unsigned short height)
{
    _realm_name = realm_name;
    _width = width;
    _height = height;

    string directory = "realm/" + _realm_name + "/"; 
    make_dir( directory.c_str() );
    string filename = directory + "base.dat";
    
    open_file( _ofs_base, filename, ios::trunc|ios::in|ios::out|ios::binary );


    MAPBLOCK empty;
    memset( &empty, 0, sizeof empty );
    for( unsigned i = 0; i < total_blocks(); ++i )
    {
        _ofs_base.write( reinterpret_cast<const char*>(&empty), sizeof empty );
    }

    // First-level Solids index (solidx1)
    filename = directory + "solidx1.dat";
    open_file( _ofs_solidx1, filename, ios::trunc|ios::in|ios::out|ios::binary );
    SOLIDX1_ELEM elem;
    elem.offset = 0;
    for( unsigned i = 0; i < total_solid_blocks(); ++i )
    {
        _ofs_solidx1.write( reinterpret_cast<const char*>(&elem), sizeof elem );
    }

    // Second-level Solids index (solidx2)
    filename = directory + "solidx2.dat";
    open_file( _ofs_solidx2, filename, ios::trunc|ios::in|ios::out|ios::binary );
    _ofs_solidx2.write( "fill", 4 );

    // Solids data (solids)
    filename = directory + "solids.dat";
    open_file( _ofs_solids, filename, ios::trunc|ios::in|ios::out|ios::binary );
    _ofs_solids.write( "filler", 6 ); // multiple of 3


    // Maptile file (maptile.dat)
    filename = directory + "maptile.dat";
    open_file( _ofs_maptile, filename, ios::trunc|ios::in|ios::out|ios::binary );
    MAPTILE_BLOCK maptile_empty;
    memset( &maptile_empty, 0, sizeof maptile_empty );
    for( unsigned i = 0; i < total_maptile_blocks(); ++i )
    {
        _ofs_maptile.write( reinterpret_cast<const char*>(&maptile_empty), sizeof maptile_empty );
    }
}

void MapWriter::OpenExistingFiles( const string& realm_name )
{
    _realm_name = realm_name;

    string directory = "realm/" + _realm_name + "/"; 

    string realm_cfg_filename = directory + "realm.cfg";
    ConfigFile cf( realm_cfg_filename, "REALM" );
    ConfigElem elem;
    if (!cf.read( elem ))
        throw runtime_error( "Unable to read realm from " + realm_cfg_filename );
    _width = elem.remove_ushort( "width" );
    _height = elem.remove_ushort( "height" );

    string filename = directory + "base.dat";
    open_file( _ofs_base, filename, ios::in|ios::out|ios::binary );


    // First-level Solids index (solidx1)
    filename = directory + "solidx1.dat";
    open_file( _ofs_solidx1, filename, ios::in|ios::out|ios::binary );

    // Second-level Solids index (solidx2)
    filename = directory + "solidx2.dat";
    open_file( _ofs_solidx2, filename, ios::in|ios::out|ios::binary );
    _ofs_solidx2.seekp( 0, ios::end );

    // Solids data (solids)
    filename = directory + "solids.dat";
    open_file( _ofs_solids, filename, ios::in|ios::out|ios::binary );
    _ofs_solids.seekp( 0, ios::end );

    // Maptile (maptile.dat)
    filename = directory + "maptile.dat";
    open_file( _ofs_maptile, filename, ios::in|ios::out|ios::binary );
    _ofs_maptile.seekp( 0, ios::end );
}

MapWriter::~MapWriter()
{
    Flush();
}

void MapWriter::Flush()
{
    FlushBaseFile();
    FlushMapTileFile();
}

std::fstream::pos_type MapWriter::total_size()
{
    return total_blocks() * sizeof(MAPBLOCK);
}
unsigned int MapWriter::total_blocks()
{
    return _width * _height / MAPBLOCK_CHUNK / MAPBLOCK_CHUNK;
}
unsigned int MapWriter::total_solid_blocks()
{
    return _width * _height / SOLIDX_X_SIZE / SOLIDX_Y_SIZE;
}
unsigned int MapWriter::total_maptile_blocks()
{
    return _width * _height / MAPTILE_CHUNK / MAPTILE_CHUNK;
}

void MapWriter::SetMapCell( unsigned short x, unsigned short y, MAPCELL cell )
{
    unsigned short xblock = x >>  MAPBLOCK_SHIFT;
    unsigned short xcell  = x &   MAPBLOCK_CELLMASK;
    unsigned short yblock = y >>  MAPBLOCK_SHIFT;
    unsigned short ycell  = y &   MAPBLOCK_CELLMASK;

    // doh, need to know map geometry here.
    int blockIdx = yblock * (_width >> MAPBLOCK_SHIFT) + xblock;
    if (blockIdx != _cur_mapblock_index)
    {
        if (_cur_mapblock_index >= 0)
        {
            FlushBaseFile();
        }
        // read the existing block in
        size_t offset = blockIdx * sizeof(_block);
        _ofs_base.seekg( offset, std::ios_base::beg ); 
        _ofs_base.read( reinterpret_cast<char*>(&_block), sizeof _block );
        _cur_mapblock_index = blockIdx;
    }

    _block.cell[xcell][ycell] = cell;
}
void MapWriter::SetMapTile( unsigned short x, unsigned short y, MAPTILE_CELL cell )
{
    unsigned short xblock = x >> MAPTILE_SHIFT;
    unsigned short xcell  = x &  MAPTILE_CELLMASK;
    unsigned short yblock = y >> MAPTILE_SHIFT;
    unsigned short ycell  = y &  MAPTILE_CELLMASK;

    // doh, need to know map geometry here.
    int blockIdx = yblock * (_width >> MAPTILE_SHIFT) + xblock;
    if (blockIdx != _cur_maptile_index)
    {
        if (_cur_maptile_index >= 0)
        {
            FlushMapTileFile();
        }
        // read the existing block in
        size_t offset = blockIdx * sizeof _maptile_block;
        _ofs_maptile.seekg( offset, std::ios_base::beg ); 
        _ofs_maptile.read( reinterpret_cast<char*>(&_maptile_block), sizeof _maptile_block );
        _cur_maptile_index = blockIdx;
    }

    _maptile_block.cell[xcell][ycell] = cell;
}
void MapWriter::FlushBaseFile()
{
    if (_cur_mapblock_index >= 0)
    {
        size_t offset = _cur_mapblock_index * sizeof(_block);
        _ofs_base.seekp( offset, std::ios_base::beg );
        _ofs_base.write( reinterpret_cast<const char*>(&_block), sizeof _block );
    }
}
void MapWriter::FlushMapTileFile()
{
    if (_cur_maptile_index >= 0)
    {
        size_t offset = _cur_maptile_index * sizeof(_maptile_block);
        _ofs_maptile.seekp( offset, std::ios_base::beg );
        _ofs_maptile.write( reinterpret_cast<const char*>(&_maptile_block), sizeof _maptile_block );
    }
}

unsigned int MapWriter::NextSolidOffset()
{
    return static_cast<unsigned int>(_ofs_solids.tellp());
}

unsigned int MapWriter::NextSolidIndex()
{
    return NextSolidOffset() / sizeof(SOLIDS_ELEM);
}

unsigned int MapWriter::NextSolidx2Offset()
{
    return static_cast<unsigned int>(_ofs_solidx2.tellp());
}

void MapWriter::AppendSolid( const SOLIDS_ELEM& solid )
{
    _ofs_solids.write( reinterpret_cast<const char*>(&solid), sizeof(SOLIDS_ELEM) );
}

void MapWriter::AppendSolidx2Elem( const SOLIDX2_ELEM& elem )
{
    _ofs_solidx2.write( reinterpret_cast<const char*>( &elem ), sizeof elem );
}

void MapWriter::SetSolidx2Offset( unsigned short x_base, unsigned short y_base, unsigned int offset )
{
    unsigned int elems_per_row = (_width / SOLIDX_X_SIZE);
    unsigned int index = (y_base / SOLIDX_Y_SIZE) * elems_per_row
                         +(x_base / SOLIDX_X_SIZE);
    size_t file_offset = index * sizeof(SOLIDX1_ELEM);

    _ofs_solidx1.seekp( file_offset, std::ios_base::beg );
    _ofs_solidx1.write( reinterpret_cast<const char*>(&offset), sizeof offset);
}
