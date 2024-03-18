/** @file
 *
 * @par History
 */


#include "mapwriter.h"

#include <string.h>

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/iohelp.h"
#include "mapcell.h"
#include "mapsolid.h"
#include "realmdescriptor.h"
#include "uofile.h"


namespace Pol
{
namespace Plib
{
extern unsigned int num_map_patches;
extern unsigned int num_static_patches;

MapWriter::MapWriter()
    : _realm_name( "" ),
      _width( 0 ),
      _height( 0 ),
      _ofs_base(),
      _cur_mapblock_index( -1 ),
      _ofs_solidx1(),
      _ofs_solidx2(),
      _ofs_solids(),
      _ofs_maptile(),
      _cur_maptile_index( -1 ),
      solidx2_offset( 0 ),
      solids_offset( 0 )
{
  _ofs_base.exceptions( std::ios_base::failbit | std::ios_base::badbit );
  _ofs_solidx1.exceptions( std::ios_base::failbit | std::ios_base::badbit );
  _ofs_solidx2.exceptions( std::ios_base::failbit | std::ios_base::badbit );
  _ofs_solids.exceptions( std::ios_base::failbit | std::ios_base::badbit );
  _ofs_maptile.exceptions( std::ios_base::failbit | std::ios_base::badbit );
}

void MapWriter::WriteConfigFile()
{
  std::string filename = "realm/" + _realm_name + "/realm.cfg";
  std::ofstream ofs_cfg;
  ofs_cfg.exceptions( std::ios_base::failbit | std::ios_base::badbit );

  Clib::open_file( ofs_cfg, filename, std::ios::trunc | std::ios::out );

  ofs_cfg << "Realm " << _realm_name << std::endl;
  ofs_cfg << "{" << std::endl;
  ofs_cfg << "    width " << _width << std::endl;
  ofs_cfg << "    height " << _height << std::endl;
  ofs_cfg << "    mapserver memory" << std::endl;
  ofs_cfg << "    uomapid " << uo_mapid << std::endl;
  ofs_cfg << "    uodif " << uo_usedif << std::endl;
  ofs_cfg << "    num_static_patches " << num_static_patches << std::endl;
  ofs_cfg << "    num_map_patches " << num_map_patches << std::endl;
  ofs_cfg << "    season 1" << std::endl;
  ofs_cfg << "    version " << RealmDescriptor::VERSION << std::endl;
  ofs_cfg << "}" << std::endl;
}

void MapWriter::CreateBaseDat( const std::string& /*realm_name*/, const std::string& directory )
{
  using std::ios;

  std::string filename = directory + "base.dat";
  Clib::open_file( _ofs_base, filename, ios::trunc | ios::in | ios::out | ios::binary );
  MAPBLOCK empty;
  memset( &empty, 0, sizeof empty );
  for ( unsigned i = 0; i < total_blocks(); ++i )
  {
    _ofs_base.write( reinterpret_cast<const char*>( &empty ), sizeof empty );
  }
}
void MapWriter::CreateSolidx1Dat( const std::string& /*realm_name*/, const std::string& directory )
{
  using std::ios;

  std::string filename = directory + "solidx1.dat";
  Clib::open_file( _ofs_solidx1, filename, ios::trunc | ios::in | ios::out | ios::binary );
  SOLIDX1_ELEM elem;
  elem.offset = 0;
  for ( unsigned i = 0; i < total_solid_blocks(); ++i )
  {
    _ofs_solidx1.write( reinterpret_cast<const char*>( &elem ), sizeof elem );
  }
}
void MapWriter::CreateSolidx2Dat( const std::string& /*realm_name*/, const std::string& directory )
{
  using std::ios;
  std::string filename = directory + "solidx2.dat";
  Clib::open_file( _ofs_solidx2, filename, ios::trunc | ios::in | ios::out | ios::binary );
  _ofs_solidx2.write( "fill", 4 );
  solidx2_offset = 4;
}
void MapWriter::CreateSolidsDat( const std::string& /*realm_name*/, const std::string& directory )
{
  using std::ios;

  std::string filename = directory + "solids.dat";
  Clib::open_file( _ofs_solids, filename, ios::trunc | ios::in | ios::out | ios::binary );
  _ofs_solids.write( "filler", 6 );  // multiple of 3
  solids_offset = 6;
}
void MapWriter::CreateMaptileDat( const std::string& /*realm_name*/, const std::string& directory )
{
  using std::ios;

  std::string filename = directory + "maptile.dat";
  Clib::open_file( _ofs_maptile, filename, ios::trunc | ios::in | ios::out | ios::binary );
  MAPTILE_BLOCK maptile_empty;
  memset( &maptile_empty, 0, sizeof maptile_empty );
  for ( unsigned i = 0; i < total_maptile_blocks(); ++i )
  {
    _ofs_maptile.write( reinterpret_cast<const char*>( &maptile_empty ), sizeof maptile_empty );
  }
}

void MapWriter::CreateNewFiles( const std::string& realm_name, unsigned short width,
                                unsigned short height )
{
  _realm_name = realm_name;
  _width = width;
  _height = height;

  std::string directory = "realm/" + _realm_name + "/";
  Clib::make_dir( directory.c_str() );

  CreateBaseDat( realm_name, directory );
  // First-level Solids index (solidx1)
  CreateSolidx1Dat( realm_name, directory );
  // Second-level Solids index (solidx2)
  CreateSolidx2Dat( realm_name, directory );
  // Solids data (solids)
  CreateSolidsDat( realm_name, directory );
  // Maptile file (maptile.dat)
  CreateMaptileDat( realm_name, directory );
}

void MapWriter::OpenExistingFiles( const std::string& realm_name )
{
  using std::ios;

  _realm_name = realm_name;

  std::string directory = "realm/" + _realm_name + "/";

  std::string realm_cfg_filename = directory + "realm.cfg";
  Clib::ConfigFile cf( realm_cfg_filename, "REALM" );
  Clib::ConfigElem elem;
  if ( !cf.read( elem ) )
    throw std::runtime_error( "Unable to read realm from " + realm_cfg_filename );
  _width = elem.remove_ushort( "width" );
  _height = elem.remove_ushort( "height" );

  std::string filename = directory + "base.dat";
  Clib::open_file( _ofs_base, filename, ios::in | ios::out | ios::binary );


  // First-level Solids index (solidx1)
  filename = directory + "solidx1.dat";
  Clib::open_file( _ofs_solidx1, filename, ios::in | ios::out | ios::binary );

  // Second-level Solids index (solidx2)
  filename = directory + "solidx2.dat";
  Clib::open_file( _ofs_solidx2, filename, ios::in | ios::out | ios::binary );
  _ofs_solidx2.seekp( 0, ios::end );
  solidx2_offset = _ofs_solidx2.tellp();

  // Solids data (solids)
  filename = directory + "solids.dat";
  Clib::open_file( _ofs_solids, filename, ios::in | ios::out | ios::binary );
  _ofs_solids.seekp( 0, ios::end );
  solids_offset = _ofs_solids.tellp();

  // Maptile (maptile.dat)
  filename = directory + "maptile.dat";
  Clib::open_file( _ofs_maptile, filename, ios::in | ios::out | ios::binary );
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
  return total_blocks() * sizeof( MAPBLOCK );
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
  unsigned short xblock = x >> MAPBLOCK_SHIFT;
  unsigned short xcell = x & MAPBLOCK_CELLMASK;
  unsigned short yblock = y >> MAPBLOCK_SHIFT;
  unsigned short ycell = y & MAPBLOCK_CELLMASK;

  // doh, need to know map geometry here.
  int blockIdx = yblock * ( _width >> MAPBLOCK_SHIFT ) + xblock;
  if ( blockIdx != _cur_mapblock_index )
  {
    if ( _cur_mapblock_index >= 0 )
    {
      FlushBaseFile();
    }
    // read the existing block in
    size_t offset = blockIdx * sizeof( _block );
    _ofs_base.seekg( offset, std::ios_base::beg );
    _ofs_base.read( reinterpret_cast<char*>( &_block ), sizeof _block );
    _cur_mapblock_index = blockIdx;
  }

  _block.cell[xcell][ycell] = cell;
}
void MapWriter::SetMapTile( unsigned short x, unsigned short y, MAPTILE_CELL cell )
{
  unsigned short xblock = x >> MAPTILE_SHIFT;
  unsigned short xcell = x & MAPTILE_CELLMASK;
  unsigned short yblock = y >> MAPTILE_SHIFT;
  unsigned short ycell = y & MAPTILE_CELLMASK;

  // doh, need to know map geometry here.
  int blockIdx = yblock * ( _width >> MAPTILE_SHIFT ) + xblock;
  if ( blockIdx != _cur_maptile_index )
  {
    if ( _cur_maptile_index >= 0 )
    {
      FlushMapTileFile();
    }
    // read the existing block in
    size_t offset = blockIdx * sizeof _maptile_block;
    _ofs_maptile.seekg( offset, std::ios_base::beg );
    _ofs_maptile.read( reinterpret_cast<char*>( &_maptile_block ), sizeof _maptile_block );
    _cur_maptile_index = blockIdx;
  }

  _maptile_block.cell[xcell][ycell] = cell;
}
void MapWriter::FlushBaseFile()
{
  if ( _cur_mapblock_index >= 0 )
  {
    size_t offset = _cur_mapblock_index * sizeof( _block );
    _ofs_base.seekp( offset, std::ios_base::beg );
    _ofs_base.write( reinterpret_cast<const char*>( &_block ), sizeof _block );
  }
}
void MapWriter::FlushMapTileFile()
{
  if ( _cur_maptile_index >= 0 )
  {
    size_t offset = _cur_maptile_index * sizeof( _maptile_block );
    _ofs_maptile.seekp( offset, std::ios_base::beg );
    _ofs_maptile.write( reinterpret_cast<const char*>( &_maptile_block ), sizeof _maptile_block );
  }
}

unsigned int MapWriter::NextSolidOffset()
{
  return static_cast<unsigned int>( solids_offset );
}

unsigned int MapWriter::NextSolidIndex()
{
  return NextSolidOffset() / sizeof( SOLIDS_ELEM );
}

unsigned int MapWriter::NextSolidx2Offset()
{
  return static_cast<unsigned int>( solidx2_offset );
}

void MapWriter::AppendSolid( const SOLIDS_ELEM& solid )
{
  _ofs_solids.write( reinterpret_cast<const char*>( &solid ), sizeof( SOLIDS_ELEM ) );
  solids_offset += sizeof( SOLIDS_ELEM );
}

void MapWriter::AppendSolidx2Elem( const SOLIDX2_ELEM& elem )
{
  _ofs_solidx2.write( reinterpret_cast<const char*>( &elem ), sizeof elem );
  solidx2_offset += sizeof elem;
}

void MapWriter::SetSolidx2Offset( unsigned short x_base, unsigned short y_base,
                                  unsigned int offset )
{
  unsigned int elems_per_row = ( _width / SOLIDX_X_SIZE );
  unsigned int index = ( y_base / SOLIDX_Y_SIZE ) * elems_per_row + ( x_base / SOLIDX_X_SIZE );
  size_t file_offset = index * sizeof( SOLIDX1_ELEM );

  _ofs_solidx1.seekp( file_offset, std::ios_base::beg );
  _ofs_solidx1.write( reinterpret_cast<const char*>( &offset ), sizeof offset );
}
}  // namespace Plib
}  // namespace Pol
