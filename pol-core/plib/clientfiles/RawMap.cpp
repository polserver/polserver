#include "RawMap.h"

#include "clib/fileutil.h"
#include "clib/logfacility.h"
#include "clib/passert.h"
#include "clib/strutil.h"
#include "plib/mul/map.h"
#include "staticblock.h"
#include "systemstate.h"
#include "uoclientfiles.h"

#include "uopreader/uop.h"
#include "uopreader/uophash.h"

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <map>
#include <span>
#include <stdexcept>
#include <string>


namespace Pol::Plib
{
// This code is almost identical to the one in RawMap::load_full_map. One should consider a way to
// refactor both.
static size_t uop_equivalent_mul_size( std::ifstream& ifs, int uo_mapid )
{
  kaitai::kstream ks( &ifs );
  uop_t uopfile( &ks );
  auto maphash = []( int mapid, size_t chunkidx )
  {
    char mapstring[1024];
    snprintf( mapstring, sizeof mapstring, "build/map%dlegacymul/%08i.dat", mapid, (int)chunkidx );
    return HashLittle2( mapstring );
  };

  size_t totalSize = 0;
  unsigned int nreadfiles = 0;
  std::map<uint64_t, size_t> fileSizes;

  uop_t::block_addr_t* currentblock = uopfile.header()->firstblock();
  do
  {
    if ( currentblock->blockaddr() == 0 )
      break;
    if ( currentblock->block_body()->files() == nullptr )
      break;

    for ( auto file : *currentblock->block_body()->files() )
    {
      if ( file == nullptr )
        continue;
      if ( file->decompressed_size() == 0 )
        continue;

      passert_r( file->compression_type() == uop_t::COMPRESSION_TYPE_NO_COMPRESSION,
                 "This map is zlib compressed and we can't handle that yet." );

      nreadfiles++;
      fileSizes[file->filehash()] = file->decompressed_size();
    }
    currentblock = currentblock->block_body()->next_addr();
  } while ( currentblock != nullptr && nreadfiles < uopfile.header()->nfiles() );

  if ( uopfile.header()->nfiles() != nreadfiles )
    INFO_PRINTLN( "Warning: not all chunks read ({}/{})", nreadfiles, uopfile.header()->nfiles() );

  for ( size_t i = 0; i < fileSizes.size(); i++ )
  {
    auto fileitr = fileSizes.find( maphash( uo_mapid, i ) );
    if ( fileitr == fileSizes.end() )
    {
      ERROR_PRINTLN( "Couldn't find file hash: {}", std::to_string( maphash( uo_mapid, i ) ) );
      throw std::runtime_error( "UOP map is missing a file chunk." );
    }

    // Each chunk typically has 0xC4000 bytes (4096 blocks). If the chunk is smaller,
    // the size is off by one.
    const size_t chunkSize = fileitr->second;
    if ( chunkSize < 0xC4000 )
      totalSize += chunkSize - MUL::Map::blockSize;
    else
      totalSize += chunkSize;
  }

  ifs.clear();
  ifs.seekg( 0, std::ios::beg );

  return totalSize;
}

bool RawMap::open_uop( int mapid, size_t* out_file_size )
{
  std::string filepart = "map" + std::to_string( mapid ) + "LegacyMUL.uop";
  std::string filename = systemstate.config.uo_datafile_root + filepart;
  if ( !Clib::FileExists( filename ) )
  {
    INFO_PRINTLN( "{} not found in {}. Searching for old map[N].mul files.", filepart,
                  systemstate.config.uo_datafile_root );
    return false;
  }

  uopmapfile_.open( filename, std::ios::binary );

  if ( uopmapfile_.fail() )
    return false;

  if ( out_file_size != nullptr )
    *out_file_size = uop_equivalent_mul_size( uopmapfile_, mapid );
  return true;
}

size_t RawMap::open( int mapid, bool usedif, bool readuop )
{
  size_t map_size = 0;
  // First tries to load the new UOP files. Otherwise fall back to map[N].mul files.
  // map1 uses map0 + 'dif' files, unless there is a map1.mul (newer clients).
  if ( !readuop || !open_uop( mapid, &map_size ) )
    mapfile_.reset( open_map_file( "map", mapid, &map_size ) );

  if ( usedif )
  {
    std::string filename = "mapdifl" + Clib::tostring( mapid ) + ".mul";
    if ( Clib::FileExists( systemstate.config.uo_datafile_root + filename ) )
    {
      mapdifl_file_.reset( open_uo_file( filename ) );
      filename = "mapdif" + Clib::tostring( mapid ) + ".mul";
      mapdif_file_.reset( open_uo_file( filename ) );
    }
  }
  return map_size;
}

unsigned int RawMap::read_difflist()
{
  m_num_patches = load_map_difflist( mapdifl_file_ );
  return m_num_patches;
}

void RawMap::full_read( int mapid, unsigned short width, unsigned short height )
{
  set_bounds( width, height );

  unsigned int blocks = 0;
  if ( mapfile_.get() == nullptr )
    blocks = load_full_map( mapid, uopmapfile_ );
  else
    blocks = load_full_map( mapfile_, mapdif_file_ );

  if ( blocks )
    is_init = true;
}

USTRUCT_MAPINFO RawMap::get_cell( unsigned int blockidx, unsigned int x_offset,
                                  unsigned int y_offset ) const
{
  return m_mapinfo_vec.at( blockidx ).cell[y_offset][x_offset];
}

signed char RawMap::rawinfo( unsigned short x, unsigned short y, USTRUCT_MAPINFO* gi ) const
{
  passert_r( is_init, "Tried to read map information before loading the map files" );
  passert( m_mapwidth > 0 && m_mapheight > 0 && x < m_mapwidth && y < m_mapheight );

  // The client's map mul shares the column-major 8x8 block layout of staidx.
  size_t block = staticblock_from_coords( x, y, m_mapheight );

  unsigned int x_offset = x & 0x7;
  unsigned int y_offset = y & 0x7;

  *gi = get_cell( static_cast<unsigned int>( block ), x_offset, y_offset );
  return gi->z;
}

void RawMap::extract_planes( std::span<u16> landtile_out, std::span<s8> z_out ) const
{
  passert_r( is_init, "Tried to read map information before loading the map files" );
  passert( m_mapwidth > 0 && m_mapheight > 0 );
  const std::size_t n = static_cast<std::size_t>( m_mapwidth ) * m_mapheight;
  passert( landtile_out.size() == n && z_out.size() == n );

  const unsigned int hblocks = m_mapheight / 8;
  for ( unsigned int y = 0; y < m_mapheight; ++y )
  {
    const unsigned int y_block = y / 8;
    const unsigned int y_offset = y & 0x7;
    const std::size_t row = static_cast<std::size_t>( y ) * m_mapwidth;
    for ( unsigned int x = 0; x < m_mapwidth; ++x )
    {
      // Same block/cell selection as rawinfo(), but with operator[] (no bounds check)
      // and no USTRUCT_MAPINFO copy.
      const USTRUCT_MAPINFO& cell =
          m_mapinfo_vec[( x / 8 ) * hblocks + y_block].cell[y_offset][x & 0x7];
      landtile_out[row + x] = cell.landtile;
      z_out[row + x] = cell.z;
    }
  }
}

void RawMap::add_block( const USTRUCT_MAPINFO_BLOCK& block )
{
  m_mapinfo_vec.push_back( block );
}

void RawMap::set_bounds( unsigned short width, unsigned short height )
{
  m_mapwidth = width;
  m_mapheight = height;
}

unsigned int RawMap::load_map_difflist( FILE* mapdifl_file )
{
  unsigned index = 0;
  if ( mapdifl_file != nullptr )
  {
    u32 blockid;
    while ( fread( &blockid, sizeof blockid, 1, mapdifl_file ) == 1 )
    {
      mapdifl[blockid] = index;
      ++index;
    }
  }
  return index;
}

unsigned int RawMap::load_full_map( FILE* mapfile, FILE* mapdif_file )
{
  passert_r( !is_init, "tried to initialize RawMap twice" );

  if ( mapfile == nullptr )
    throw std::runtime_error( "load_full_map: mapfile is not open" );

  if ( !mapdifl.empty() && mapdif_file == nullptr )
    throw std::runtime_error( "load_full_map: mapdifl is loaded but mapdif is not" );

  // Bulk-read the whole map file once instead of one 196-byte fread per block.
  std::vector<std::byte> map_buf = Clib::ReadEntireFile( mapfile );
  std::span<const USTRUCT_MAPINFO_BLOCK> blocks(
      reinterpret_cast<const USTRUCT_MAPINFO_BLOCK*>( map_buf.data() ),
      map_buf.size() / sizeof( USTRUCT_MAPINFO_BLOCK ) );

  m_mapinfo_vec.reserve( blocks.size() );

  for ( unsigned int block = 0; block < blocks.size(); ++block )
  {
    auto citr = mapdifl.find( block );
    if ( citr == mapdifl.end() ) [[likely]]
    {
      add_block( blocks[block] );
    }
    else [[unlikely]]
    {
      // it's in the dif file.. get it from there.
      unsigned dif_index = ( *citr ).second;
      unsigned int file_offset = dif_index * sizeof( USTRUCT_MAPINFO_BLOCK );
      if ( fseek( mapdif_file, file_offset, SEEK_SET ) != 0 )
        throw std::runtime_error( "rawmapinfo: fseek(mapdif_file) failure" );

      USTRUCT_MAPINFO_BLOCK buffer;
      if ( fread( &buffer, sizeof buffer, 1, mapdif_file ) != 1 )
        throw std::runtime_error( "rawmapinfo: fread(mapdif_file) failure" );
      add_block( buffer );
    }
  }

  return static_cast<unsigned int>( blocks.size() );
}

// TODO: cleanup the code
unsigned int RawMap::load_full_map( int uo_mapid, std::istream& ifs )
{
  auto maphash = []( int mapid, size_t chunkidx )
  {
    char mapstring[1024];
    snprintf( mapstring, sizeof mapstring, "build/map%dlegacymul/%08i.dat", mapid, (int)chunkidx );
    return HashLittle2( mapstring );
  };

  passert_r( !is_init, "tried to initialize RawMap twice" );

  if ( !ifs )
  {
    ERROR_PRINTLN( "Error when opening UOP file." );
    return 0;
  }

  kaitai::kstream ks( &ifs );
  uop_t uopfile( &ks );

  size_t totalSize = 0;
  std::map<uint64_t, uop_t::file_t*> filemap;

  // First pass over the data to build up filehash -> file mapping and calculate total map size.
  uop_t::block_addr_t* currentblock = uopfile.header()->firstblock();
  do
  {
    for ( auto file : *currentblock->block_body()->files() )
    {
      if ( file == nullptr )
        continue;
      if ( file->decompressed_size() == 0 )
        continue;

      passert_r( file->compression_type() == uop_t::COMPRESSION_TYPE_NO_COMPRESSION,
                 "This map is zlib compressed and we can't handle that yet." );

      filemap[file->filehash()] = file;
      totalSize += file->decompressed_size();
    }
    currentblock = currentblock->block_body()->next_addr();
  } while ( currentblock != nullptr && filemap.size() < uopfile.header()->nfiles() );

  if ( uopfile.header()->nfiles() != filemap.size() )
    INFO_PRINTLN( "Warning: not all chunks read ({}/{})", filemap.size(),
                  uopfile.header()->nfiles() );

  // Sanity checking and pre-allocate mapinfo vector
  passert_r(
      size_t( totalSize / sizeof( USTRUCT_MAPINFO_BLOCK ) ) * sizeof( USTRUCT_MAPINFO_BLOCK ) ==
          totalSize,
      "Not an integer number of blocks! Check if the sizes are correct." );
  m_mapinfo_vec.clear();
  m_mapinfo_vec.resize( totalSize / sizeof( USTRUCT_MAPINFO_BLOCK ) );

  // Reads the mapinfo by concatenating all tiles, in the correct order, directly into the mapinfo
  // vector.
  size_t vecidx = 0;
  for ( size_t i = 0, remaining = totalSize; i < filemap.size(); i++ )
  {
    auto fileitr = filemap.find( maphash( uo_mapid, i ) );
    if ( fileitr == filemap.end() )
    {
      ERROR_PRINTLN( "Couldn't find file hash: {}", std::to_string( maphash( uo_mapid, i ) ) );
      throw std::runtime_error( "UOP map is missing a file chunk." );
    }

    auto file = fileitr->second;

    passert( remaining >= file->data()->filebytes().size() );
    memcpy( &m_mapinfo_vec[vecidx], file->data()->filebytes().data(),
            file->data()->filebytes().size() );

    remaining -= file->data()->filebytes().size();
    vecidx += file->data()->filebytes().size() / sizeof( USTRUCT_MAPINFO_BLOCK );
  }

  return (unsigned int)m_mapinfo_vec.size();
}

RawMap::RawMap() : is_init( false ), m_mapinfo_vec(), mapdifl(), m_mapwidth( 0 ), m_mapheight( 0 )
{
}

}  // namespace Pol::Plib
