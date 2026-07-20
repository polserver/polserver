/** @file
 * Opens/closes the UO client data files (map/statics/tiledata/verdata/difs),
 * including UOP map probing and decompression into the legacy mul layout.
 *
 * @par History
 */

#include "uofile.h"

#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>

#include <zlib.h>

#include "clib/fileutil.h"
#include "clib/logfacility.h"
#include "clib/passert.h"
#include "clib/rawtypes.h"
#include "clib/strutil.h"
#include "objtype.h"
#include "plib/mul/map.h"
#include "plib/mul/tiledata.h"
#include "plib/uopreader/uop.h"
#include "plib/uopreader/uophash.h"
#include "systemstate.h"
#include "uoclientfiles.h"


namespace Pol::Plib
{
UoClientFiles& uofiles()
{
  static UoClientFiles instance;
  return instance;
}

class ByteReader
{
  const unsigned char* data_ = nullptr;
  std::size_t size_ = 0;
  std::size_t pos_ = 0;

public:
  ByteReader() = default;

  void load( const unsigned char* data, std::size_t size )
  {
    data_ = data;
    size_ = size;
    pos_ = 0;
  }

  template <typename T>
  T read()
  {
    static_assert( std::is_trivially_copyable_v<T>, "Can only read trivially copyable types" );

    if ( pos_ + sizeof( T ) > size_ )
    {
      throw std::out_of_range( "ByteReader: not enough data to read" );
    }

    T value;
    std::memcpy( &value, data_ + pos_, sizeof( T ) );
    pos_ += sizeof( T );
    return value;
  }

  template <typename T>
  void skip( std::size_t count )
  {
    static_assert( std::is_trivially_copyable_v<T>, "Can only skip trivially copyable types" );

    if ( pos_ + count * sizeof( T ) > size_ )
    {
      throw std::out_of_range( "ByteReader: not enough data to skip" );
    }

    pos_ += count * sizeof( T );
  }
};


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

bool UoClientFiles::open_uopmap_file( size_t* out_file_size )
{
  std::string filepart = "map" + std::to_string( uo_mapid ) + "LegacyMUL.uop";
  std::string filename = systemstate.config.uo_datafile_root + filepart;
  if ( !Clib::FileExists( filename ) )
  {
    INFO_PRINTLN( "{} not found in {}. Searching for old map[N].mul files.", filepart,
                  systemstate.config.uo_datafile_root );
    return false;
  }

  uopmapfile.open( filename, std::ios::binary );

  if ( uopmapfile.fail() )
    return false;

  if ( out_file_size != nullptr )
    *out_file_size = uop_equivalent_mul_size( uopmapfile, uo_mapid );
  return true;
}

bool open_uopmulti_file( std::map<unsigned int, std::vector<USTRUCT_MULTI_ELEMENT>>& multi_map )
{
  const std::string filepart = "MultiCollection.uop";
  const std::string filename = systemstate.config.uo_datafile_root + filepart;
  if ( !Clib::FileExists( filename ) )
  {
    INFO_PRINTLN( "{} not found in {}. Searching for old multi.mul/multi.idx files.", filepart,
                  systemstate.config.uo_datafile_root );
    return false;
  }

  std::ifstream ifs;
  ifs.open( filename, std::ios::binary );

  if ( ifs.fail() )
    return false;

  kaitai::kstream ks( &ifs );
  uop_t uopfile( &ks );
  ByteReader reader;
  std::unique_ptr<unsigned char[]> uncompressed;

  unsigned int nreadfiles = 0;

  uop_t::block_addr_t* currentblock = uopfile.header()->firstblock();

  auto multihash = []( size_t chunkidx )
  {
    char mapstring[1024];

    snprintf( mapstring, sizeof mapstring, "build/multicollection/%06i.bin", (int)chunkidx );
    return HashLittle2( mapstring );
  };

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

      unsigned long datalen = file->decompressed_size();
      const auto& filebytes = file->data()->filebytes();

      if ( file->compression_type() == uop_t::COMPRESSION_TYPE_ZLIB )
      {
        if ( filebytes.size() > std::numeric_limits<unsigned long>::max() )
        {
          throw std::runtime_error( "File size exceeds maximum limit." );
        }

        uncompressed.reset( new unsigned char[datalen] );
        auto res = uncompress( uncompressed.get(), &datalen,
                               reinterpret_cast<const unsigned char*>( filebytes.c_str() ),
                               static_cast<unsigned long>( filebytes.size() ) );

        if ( res != Z_OK )
        {
          throw std::runtime_error( fmt::format( "Error decompressing UOP file at block address {}",
                                                 currentblock->blockaddr() ) );
        }
        reader.load( uncompressed.get(), datalen );
      }
      else
      {
        reader.load( reinterpret_cast<const unsigned char*>( filebytes.c_str() ), datalen );
      }

      auto id = reader.read<u32>();

      auto hash = file->filehash();
      auto calculated_hash = multihash( id );

      // Still need to check hash, because `file` may not be a multicollection
      // file, but we'll still count it as a read file (ie. `nreadfiles++`)
      if ( hash == calculated_hash )
      {
        auto count = reader.read<s32>();

        std::vector<USTRUCT_MULTI_ELEMENT> elems;

        for ( int j = 0; j < count; j++ )
        {
          auto graphic = reader.read<u16>();
          auto x = reader.read<s16>();
          auto y = reader.read<s16>();
          auto z = reader.read<s16>();
          bool is_static = ( reader.read<u16>() & 0xFF ) == 0;
          u32 flags = is_static ? 1 : 0;
          auto clilocsCount = reader.read<u32>();
          reader.skip<u32>( clilocsCount );

          elems.push_back( { graphic, x, y, z, flags } );
        }

        multi_map[id] = std::move( elems );
      }

      nreadfiles++;
    }
    currentblock = currentblock->block_body()->next_addr();
  } while ( currentblock != nullptr && nreadfiles < uopfile.header()->nfiles() );

  if ( uopfile.header()->nfiles() != nreadfiles )
    INFO_PRINTLN( "Warning: not all chunks read ({}/{})", nreadfiles, uopfile.header()->nfiles() );

  return true;
}

FILE* open_uo_file( const std::string& filename_part, size_t* out_file_size = nullptr )
{
  std::string filename = systemstate.config.uo_datafile_root + filename_part;
  FILE* fp = fopen( filename.c_str(), "rb" );
  if ( !fp )
  {
    ERROR_PRINTLN(
        "Unable to open UO datafile: {}\n"
        "POL.CFG specifies UODataFileRoot as '{}'.  Is this correct?\n"
        "  The following files must be present in that directory:\n"
        "      map0.mul OR map0LegacyMUL.uop\n"
        "      multi.idx\n"
        "      multi.mul\n"
        "      staidx0.mul\n"
        "      statics0.mul\n"
        "      tiledata.mul\n"
        "      verdata.mul    (optional - only if present on client install)",
        filename, systemstate.config.uo_datafile_root );

    throw std::runtime_error( "Error opening UO datafile." );
  }
  if ( out_file_size != nullptr )
  {
    *out_file_size = Clib::filesize( filename.c_str() );
  }
  return fp;
}

FILE* UoClientFiles::open_map_file( const std::string& name, size_t* out_file_size )
{
  std::string filename;

  filename = name + Clib::tostring( uo_mapid ) + ".mul";
  if ( uo_mapid == 1 && !Clib::FileExists( systemstate.config.uo_datafile_root + filename ) )
  {
    ERROR_PRINTLN( "Unable to find UO file: {}, reading {}0.mul instead.", filename, name );
    filename = name + "0.mul";
  }

  return open_uo_file( filename, out_file_size );
}

void UoClientFiles::open_tiledata()
{
  size_t tiledata_size;

  tilefile = open_uo_file( "tiledata.mul", &tiledata_size );

  // Auto-detect HSA format, find number of blocks, etc
  MUL::TiledataInfo tileinfo( tiledata_size );

  cfg_use_new_hsa_format = tileinfo.is_hsa();
  Plib::systemstate.config.max_tile_id = tileinfo.max_tile_id();

  if ( !Plib::systemstate.config.max_tile_id )
  {
    ERROR_PRINTLN(
        "\n"
        "Error reading tiledata.mul:\n"
        " - The file is either corrupted or has an "
        "unknown format.\n" );

    throw std::runtime_error( "Unknown format of tiledata.mul" );
  }

  INFO_PRINTLN(
      "Using auto-detected parameters:\n"
      "\tUseNewHSAFormat = {}\n"
      "\tMaxTileID = {:#x}",
      cfg_use_new_hsa_format, Plib::systemstate.config.max_tile_id );
}

void UoClientFiles::open_map()
{
  size_t map_size;
  // First tries to load the new UOP files. Otherwise fall back to map[N].mul files.
  // map1 uses map0 + 'dif' files, unless there is a map1.mul (newer clients)
  // same for staidx and statics

  if ( !uo_readuop || !open_uopmap_file( &map_size ) )
    mapfile = open_map_file( "map", &map_size );

  uo_map_size = map_size;
}

void UoClientFiles::open_uo_data_files()
{
  open_map();

  sidxfile = open_map_file( "staidx" );
  statfile = open_map_file( "statics" );

  if ( Clib::FileExists( ( systemstate.config.uo_datafile_root + "verdata.mul" ).c_str() ) )
  {
    verfile = open_uo_file( "verdata.mul" );
  }
  else
  {
    verfile = nullptr;
  }
  open_tiledata();

  if ( uo_usedif )
  {
    std::string filename;
    filename = "stadifl" + Clib::tostring( uo_mapid ) + ".mul";
    if ( Clib::FileExists( systemstate.config.uo_datafile_root + filename ) )
    {
      stadifl_file = open_uo_file( filename );
      filename = "stadifi" + Clib::tostring( uo_mapid ) + ".mul";
      stadifi_file = open_uo_file( filename );
      filename = "stadif" + Clib::tostring( uo_mapid ) + ".mul";
      stadif_file = open_uo_file( filename );
    }
    filename = "mapdifl" + Clib::tostring( uo_mapid ) + ".mul";
    if ( Clib::FileExists( systemstate.config.uo_datafile_root + filename ) )
    {
      mapdifl_file = open_uo_file( filename );
      filename = "mapdif" + Clib::tostring( uo_mapid ) + ".mul";
      mapdif_file = open_uo_file( filename );
    }
  }
}
}  // namespace Pol::Plib
