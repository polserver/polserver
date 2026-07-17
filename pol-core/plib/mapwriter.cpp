/** @file
 *
 * @par History
 */


#include "mapwriter.h"

#include <fstream>
#include <stdexcept>

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/iohelp.h"
#include "../clib/passert.h"
#include "mapcell.h"
#include "realmdescriptor.h"
#include "uofile.h"


namespace Pol::Plib
{
extern unsigned int num_map_patches;
extern unsigned int num_static_patches;

namespace
{
// Write an entire buffer to `path`, truncating any existing file. Throws with
// the filename in the message on any open/write failure.
template <typename T>
void write_file( std::string path, const std::vector<T>& buf )
{
  std::ofstream ofs;
  ofs.exceptions( std::ios_base::failbit | std::ios_base::badbit );
  Clib::open_file( ofs, path, std::ios::out | std::ios::trunc | std::ios::binary );
  if ( !buf.empty() )
    ofs.write( reinterpret_cast<const char*>( buf.data() ),
               static_cast<std::streamsize>( buf.size() * sizeof( T ) ) );
}

// Read the entire file at `path` into `buf`, resizing it to fit. Throws with
// the filename in the message on any open/read failure, or if the file size is
// not a whole number of T elements.
template <typename T>
void read_file( std::string path, std::vector<T>& buf )
{
  std::ifstream ifs;
  ifs.exceptions( std::ios_base::failbit | std::ios_base::badbit );
  Clib::open_file( ifs, path, std::ios::in | std::ios::binary );
  ifs.seekg( 0, std::ios::end );
  const auto bytes = static_cast<std::streamoff>( ifs.tellg() );
  ifs.seekg( 0, std::ios::beg );
  if ( bytes % static_cast<std::streamoff>( sizeof( T ) ) != 0 )
    throw std::runtime_error( "File " + path + " size is not a multiple of its element size" );
  buf.resize( static_cast<size_t>( bytes ) / sizeof( T ) );
  if ( bytes )
    ifs.read( reinterpret_cast<char*>( buf.data() ), bytes );
}

// Append the raw bytes of a trivially-copyable value to a byte buffer.
template <typename T>
void append_pod( std::vector<char>& buf, const T& value )
{
  const char* p = reinterpret_cast<const char*>( &value );
  buf.insert( buf.end(), p, p + sizeof( T ) );
}

// Write `buf` to `path` only if it was modified since it was created/loaded,
// then clear the flag so a later Flush() (e.g. from the destructor) is a no-op.
template <typename T>
void flush_buffer( const std::string& path, const std::vector<T>& buf, bool& dirty )
{
  if ( !dirty )
    return;
  write_file( path, buf );
  dirty = false;
}
}  // namespace

void MapWriter::WriteConfigFile()
{
  std::string filename = _directory + "realm.cfg";
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

void MapWriter::CreateNewFiles( const std::string& realm_name, unsigned short width,
                                unsigned short height )
{
  _realm_name = realm_name;
  _width = width;
  _height = height;
  _directory = "realm/" + _realm_name + "/";
  Clib::make_dir( _directory.c_str() );

  // Zero-initialized blocks match the byte content the old block-by-block
  // zero-fill produced; the fillers keep offset 0 from referencing real data.
  _base.assign( total_blocks(), MAPBLOCK{} );
  _solidx1.assign( total_solid_blocks(), SOLIDX1_ELEM{ 0 } );
  _solidx2 = { 'f', 'i', 'l', 'l' };
  _solids = { 'f', 'i', 'l', 'l', 'e', 'r' };  // multiple of 3
  _maptile.assign( total_maptile_blocks(), MAPTILE_BLOCK{} );

  // These are brand-new files; every buffer must be written even if the
  // conversion never touches it (e.g. maptile.dat during a plain `map` run).
  _base_dirty = _solidx1_dirty = _solidx2_dirty = _solids_dirty = _maptile_dirty = true;
}

void MapWriter::OpenExistingFiles( const std::string& realm_name )
{
  _realm_name = realm_name;
  _directory = "realm/" + _realm_name + "/";

  std::string realm_cfg_filename = _directory + "realm.cfg";
  Clib::ConfigFile cf( realm_cfg_filename, "REALM" );
  Clib::ConfigElem elem;
  if ( !cf.read( elem ) )
    throw std::runtime_error( "Unable to read realm from " + realm_cfg_filename );
  _width = elem.remove_ushort( "width" );
  _height = elem.remove_ushort( "height" );

  // Round-trip the existing files through the buffers: modes that only patch
  // one file (create_maptile, update_map) leave the rest byte-identical, and
  // update_map's append-to-existing-solids behavior falls out of loading the
  // append buffers with the current file contents.
  read_file( _directory + "base.dat", _base );
  read_file( _directory + "solidx1.dat", _solidx1 );
  read_file( _directory + "solidx2.dat", _solidx2 );
  read_file( _directory + "solids.dat", _solids );
  read_file( _directory + "maptile.dat", _maptile );
}

MapWriter::~MapWriter()
{
  Flush();
}

void MapWriter::Flush()
{
  // Only dirtied buffers are written; flush_buffer clears each flag so the
  // destructor's Flush() after an explicit one writes nothing.
  flush_buffer( _directory + "base.dat", _base, _base_dirty );
  flush_buffer( _directory + "solidx1.dat", _solidx1, _solidx1_dirty );
  flush_buffer( _directory + "solidx2.dat", _solidx2, _solidx2_dirty );
  flush_buffer( _directory + "solids.dat", _solids, _solids_dirty );
  flush_buffer( _directory + "maptile.dat", _maptile, _maptile_dirty );
}

unsigned int MapWriter::total_blocks() const
{
  return _width * _height / MAPBLOCK_CHUNK / MAPBLOCK_CHUNK;
}
unsigned int MapWriter::total_solid_blocks() const
{
  return _width * _height / SOLIDX_X_SIZE / SOLIDX_Y_SIZE;
}
unsigned int MapWriter::total_maptile_blocks() const
{
  return maptile_blocks_across( _width ) * maptile_blocks_across( _height );
}

void MapWriter::SetMapCell( unsigned short x, unsigned short y, MAPCELL cell )
{
  unsigned short xblock = x >> MAPBLOCK_SHIFT;
  unsigned short xcell = x & MAPBLOCK_CELLMASK;
  unsigned short yblock = y >> MAPBLOCK_SHIFT;
  unsigned short ycell = y & MAPBLOCK_CELLMASK;

  // doh, need to know map geometry here.
  int blockIdx = yblock * ( _width >> MAPBLOCK_SHIFT ) + xblock;
  _base[blockIdx].cell[xcell][ycell] = cell;
  _base_dirty = true;
}
void MapWriter::SetMapBlock( unsigned short x_base, unsigned short y_base, const MAPBLOCK& block )
{
  passert( ( x_base & MAPBLOCK_CELLMASK ) == 0 && ( y_base & MAPBLOCK_CELLMASK ) == 0 );
  unsigned short xblock = x_base >> MAPBLOCK_SHIFT;
  unsigned short yblock = y_base >> MAPBLOCK_SHIFT;
  int blockIdx = yblock * ( _width >> MAPBLOCK_SHIFT ) + xblock;
  _base[blockIdx] = block;
  _base_dirty = true;
}

void MapWriter::SetMapTile( unsigned short x, unsigned short y, MAPTILE_CELL cell )
{
  unsigned short xblock = x >> MAPTILE_SHIFT;
  unsigned short xcell = x & MAPTILE_CELLMASK;
  unsigned short yblock = y >> MAPTILE_SHIFT;
  unsigned short ycell = y & MAPTILE_CELLMASK;

  // doh, need to know map geometry here.
  int blockIdx = yblock * maptile_blocks_across( _width ) + xblock;
  _maptile[blockIdx].cell[xcell][ycell] = cell;
  _maptile_dirty = true;
}

unsigned int MapWriter::NextSolidOffset() const
{
  return static_cast<unsigned int>( _solids.size() );
}

unsigned int MapWriter::NextSolidIndex() const
{
  return NextSolidOffset() / sizeof( SOLIDS_ELEM );
}

unsigned int MapWriter::NextSolidx2Offset() const
{
  return static_cast<unsigned int>( _solidx2.size() );
}

void MapWriter::AppendSolid( const SOLIDS_ELEM& solid )
{
  append_pod( _solids, solid );
  _solids_dirty = true;
}

void MapWriter::AppendSolidx2Elem( const SOLIDX2_ELEM& elem )
{
  append_pod( _solidx2, elem );
  _solidx2_dirty = true;
}

void MapWriter::SetSolidx2Offset( unsigned short x_base, unsigned short y_base,
                                  unsigned int offset )
{
  unsigned int elems_per_row = ( _width / SOLIDX_X_SIZE );
  unsigned int index = ( y_base / SOLIDX_Y_SIZE ) * elems_per_row + ( x_base / SOLIDX_X_SIZE );
  _solidx1[index].offset = offset;
  _solidx1_dirty = true;
}
}  // namespace Pol::Plib
