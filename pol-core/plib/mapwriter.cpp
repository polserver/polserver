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


namespace Pol::Plib
{
namespace
{
// The filler prefixes keep offset 0 from referencing real data. solids.dat's is
// two whole SOLIDS_ELEMs (NextSolidIndex counts elements, filler included);
// solidx2.dat's is 4 raw bytes -- NOT a whole 132-byte SOLIDX2_ELEM -- which is
// fine because solidx1 records byte offsets into solidx2, and is exactly why
// OutputFile models fillers as bytes rather than elements.
constexpr std::string_view SOLIDS_FILLER = "filler";
constexpr std::string_view SOLIDX2_FILLER = "fill";
static_assert( SOLIDS_FILLER.size() == 2 * sizeof( SOLIDS_ELEM ) );
static_assert( SOLIDX2_FILLER.size() == SOLIDX2_FILLER_SIZE );
}  // namespace

template <typename T>
OutputFile<T>::OutputFile( std::string name, std::string_view filler )
    : name_( std::move( name ) ), filler_( filler )
{
}

template <typename T>
void OutputFile<T>::load( const std::string& dir )
{
  std::string path = dir + name_;
  std::ifstream ifs;
  ifs.exceptions( std::ios_base::failbit | std::ios_base::badbit );
  Clib::open_file( ifs, path, std::ios::in | std::ios::binary );
  ifs.seekg( 0, std::ios::end );
  const auto bytes = static_cast<std::streamoff>( ifs.tellg() );
  ifs.seekg( 0, std::ios::beg );

  const auto filler_bytes = static_cast<std::streamoff>( filler_.size() );
  if ( bytes < filler_bytes ||
       ( bytes - filler_bytes ) % static_cast<std::streamoff>( sizeof( T ) ) != 0 )
    throw std::runtime_error( "File " + path +
                              " size is not the filler plus a whole number of elements" );

  if ( filler_bytes != 0 )
  {
    std::string prefix( filler_.size(), '\0' );
    ifs.read( prefix.data(), filler_bytes );
    if ( prefix != filler_ )
      throw std::runtime_error( "File " + path + " does not start with the expected filler" );
  }

  elems_.resize( static_cast<size_t>( ( bytes - filler_bytes ) / sizeof( T ) ) );
  if ( !elems_.empty() )
    ifs.read( reinterpret_cast<char*>( elems_.data() ),
              static_cast<std::streamsize>( elems_.size() * sizeof( T ) ) );
  dirty_ = false;
}

template <typename T>
void OutputFile<T>::store( const std::string& dir )
{
  if ( !dirty_ )
    return;
  std::string path = dir + name_;
  std::ofstream ofs;
  ofs.exceptions( std::ios_base::failbit | std::ios_base::badbit );
  Clib::open_file( ofs, path, std::ios::out | std::ios::trunc | std::ios::binary );
  if ( !filler_.empty() )
    ofs.write( filler_.data(), static_cast<std::streamsize>( filler_.size() ) );
  if ( !elems_.empty() )
    ofs.write( reinterpret_cast<const char*>( elems_.data() ),
               static_cast<std::streamsize>( elems_.size() * sizeof( T ) ) );
  dirty_ = false;
}

template class OutputFile<MAPBLOCK>;
template class OutputFile<SOLIDX1_ELEM>;
template class OutputFile<SOLIDX2_ELEM>;
template class OutputFile<SOLIDS_ELEM>;
template class OutputFile<MAPTILE_BLOCK>;

MapWriter::MapWriter()
    : _base( "base.dat" ),
      _solidx1( "solidx1.dat" ),
      _solidx2( "solidx2.dat", SOLIDX2_FILLER ),
      _solids( "solids.dat", SOLIDS_FILLER ),
      _maptile( "maptile.dat" )
{
}

MapWriter::~MapWriter()
{
  Flush();
}

void MapWriter::WriteConfigFile( int uo_mapid, int uo_usedif, unsigned int num_static_patches,
                                 unsigned int num_map_patches )
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
  // zero-fill produced.
  _base.elems().assign( total_blocks(), MAPBLOCK{} );
  _solidx1.elems().assign( total_solid_blocks(), SOLIDX1_ELEM{ 0 } );
  _solidx2.elems().clear();
  _solids.elems().clear();
  _maptile.elems().assign( total_maptile_blocks(), MAPTILE_BLOCK{} );

  // These are brand-new files; every one must be written even if the
  // conversion never touches it (e.g. maptile.dat during a plain `map` run).
  _base.mark_fresh();
  _solidx1.mark_fresh();
  _solidx2.mark_fresh();
  _solids.mark_fresh();
  _maptile.mark_fresh();
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
  _base.load( _directory );
  _solidx1.load( _directory );
  _solidx2.load( _directory );
  _solids.load( _directory );
  _maptile.load( _directory );
}

void MapWriter::Flush()
{
  // Only dirtied buffers are written; store() clears each flag so the
  // destructor's Flush() after an explicit one writes nothing.
  _base.store( _directory );
  _solidx1.store( _directory );
  _solidx2.store( _directory );
  _solids.store( _directory );
  _maptile.store( _directory );
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
  unsigned short xcell = x & MAPBLOCK_CELLMASK;
  unsigned short ycell = y & MAPBLOCK_CELLMASK;

  _base.elems()[realm_block_index( x, y, _width )].cell[xcell][ycell] = cell;
}
void MapWriter::SetMapBlock( unsigned short x_base, unsigned short y_base, const MAPBLOCK& block )
{
  passert( ( x_base & MAPBLOCK_CELLMASK ) == 0 && ( y_base & MAPBLOCK_CELLMASK ) == 0 );
  _base.elems()[realm_block_index( x_base, y_base, _width )] = block;
}

void MapWriter::SetMapTile( unsigned short x, unsigned short y, MAPTILE_CELL cell )
{
  unsigned short xcell = x & MAPTILE_CELLMASK;
  unsigned short ycell = y & MAPTILE_CELLMASK;

  _maptile.elems()[maptile_index( x, y, _width )].cell[xcell][ycell] = cell;
}

unsigned int MapWriter::NextSolidOffset() const
{
  return _solids.byte_size();
}

unsigned int MapWriter::NextSolidIndex() const
{
  // Element index, counting the two filler elements at the front of the file.
  return NextSolidOffset() / sizeof( SOLIDS_ELEM );
}

unsigned int MapWriter::NextSolidx2Offset() const
{
  return _solidx2.byte_size();
}

void MapWriter::AppendSolid( const SOLIDS_ELEM& solid )
{
  _solids.elems().push_back( solid );
}

void MapWriter::AppendSolidx2Elem( const SOLIDX2_ELEM& elem )
{
  _solidx2.elems().push_back( elem );
}

void MapWriter::SetSolidx2Offset( unsigned short x_base, unsigned short y_base,
                                  unsigned int offset )
{
  // Solid blocks share the 8x8 row-major realm block layout.
  _solidx1.elems()[realm_block_index( x_base, y_base, _width )].offset = offset;
}
}  // namespace Pol::Plib
