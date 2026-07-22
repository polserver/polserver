/** @file
 * Opens/closes the UO client data files (map/statics/tiledata/verdata/difs),
 * including UOP map probing and decompression into the legacy mul layout.
 *
 * @par History
 */

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
#include "plib/uopreader/uop.h"
#include "plib/uopreader/uophash.h"
#include "systemstate.h"
#include "uoclientfiles.h"


namespace Pol::Plib
{
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

FILE* open_uo_file( const std::string& filename_part, size_t* out_file_size )
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

FILE* open_map_file( const std::string& name, int mapid, size_t* out_file_size )
{
  std::string filename;

  filename = name + Clib::tostring( mapid ) + ".mul";
  if ( mapid == 1 && !Clib::FileExists( systemstate.config.uo_datafile_root + filename ) )
  {
    ERROR_PRINTLN( "Unable to find UO file: {}, reading {}0.mul instead.", filename, name );
    filename = name + "0.mul";
  }

  return open_uo_file( filename, out_file_size );
}

void UoClientFiles::open_uo_data_files()
{
  // Opens map<mapid>.mul or the UOP (+ mapdif* patch files when usedif).
  uo_map_size = rawmap_.open( uo_mapid, uo_usedif, uo_readuop );

  // Opens staidx/statics.mul (+ stadif* patch files when usedif).
  statics_.open( uo_mapid, uo_usedif );

  // Opens verdata.mul (if present) then tiledata.mul; HSA-detects.
  tiledata_.open();
}

// The second half of setup: fill the caches from the files opened above. Reads the
// verdata index + tiledata/landtile caches (tiledatacache), then the statics- and
// map-diff lists that the statics/rawmap loads consult.
void UoClientFiles::read_uo_data()
{
  tiledata_.load();  // verdata index, tiledata + landtile caches
  statics_.read_difflist();
  rawmap_.read_difflist();
}
}  // namespace Pol::Plib
