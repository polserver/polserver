/** @file
 *
 * @par History
 */

#include "uofile.h"

#include <cstdio>
#include <fstream>
#include <string>

#include "clib/fileutil.h"
#include "clib/logfacility.h"
#include "clib/passert.h"
#include "clib/strutil.h"
#include "plib/mul/map.h"
#include "plib/mul/tiledata.h"
#include "plib/uopreader/uop.h"
#include "plib/uopreader/uophash.h"
#include "pol/objtype.h"
#include "systemstate.h"


namespace Pol
{
namespace Plib
{
FILE* mapfile = nullptr;
FILE* sidxfile = nullptr;
FILE* statfile = nullptr;
FILE* verfile = nullptr;
FILE* tilefile = nullptr;
FILE* stadifl_file = nullptr;
FILE* stadifi_file = nullptr;
FILE* stadif_file = nullptr;
FILE* mapdifl_file = nullptr;
FILE* mapdif_file = nullptr;

std::ifstream uopmapfile;


// This code is almost identical to the one in RawMap::load_full_map. One should consider a way to
// refactor both.
size_t uop_equivalent_mul_size( std::ifstream& ifs )
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
      ERROR_PRINT << "Couldn't find file hash: " << std::to_string( maphash( uo_mapid, i ) )
                  << "\n";
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

bool open_uopmap_file( const int mapid, size_t* out_file_size = nullptr )
{
  std::string filepart = "map" + std::to_string( mapid ) + "LegacyMUL.uop";
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
    *out_file_size = uop_equivalent_mul_size( uopmapfile );
  return true;
}

FILE* open_uo_file( const std::string& filename_part, size_t* out_file_size = nullptr )
{
  std::string filename = systemstate.config.uo_datafile_root + filename_part;
  FILE* fp = fopen( filename.c_str(), "rb" );
  if ( !fp )
  {
    ERROR_PRINT << "Unable to open UO datafile: " << filename << "\n"
                << "POL.CFG specifies UODataFileRoot as '" << systemstate.config.uo_datafile_root
                << "'.  Is this correct?\n"
                << "  The following files must be present in that directory:\n"
                << "      map0.mul OR map0LegacyMUL.uop\n"
                << "      multi.idx\n"
                << "      multi.mul\n"
                << "      staidx0.mul\n"
                << "      statics0.mul\n"
                << "      tiledata.mul\n"
                << "      verdata.mul    (optional - only if present on client install)\n";

    throw std::runtime_error( "Error opening UO datafile." );
  }
  if ( out_file_size != nullptr )
  {
    *out_file_size = Clib::filesize( filename.c_str() );
  }
  return fp;
}

FILE* open_map_file( std::string name, int map_id, size_t* out_file_size = nullptr )
{
  std::string filename;

  filename = name + Clib::tostring( map_id ) + ".mul";
  if ( uo_mapid == 1 && !Clib::FileExists( systemstate.config.uo_datafile_root + filename ) )
  {
    ERROR_PRINT << "Unable to find UO file: " << filename
                << ", reading " + name + "0.mul instead.\n";
    filename = name + "0.mul";
  }

  return open_uo_file( filename, out_file_size );
}

int uo_mapid = 0;
int uo_usedif = 1;
bool uo_readuop = true;

unsigned short uo_map_width = 0;
unsigned short uo_map_height = 0;
size_t uo_map_size = 0;

void open_tiledata( void )
{
  size_t tiledata_size;

  tilefile = open_uo_file( "tiledata.mul", &tiledata_size );

  // Auto-detect HSA format, find number of blocks, etc
  MUL::TiledataInfo tileinfo( tiledata_size );

  // Save the parameters into this ugly global state we have
  Plib::cfg_use_new_hsa_format = tileinfo.is_hsa();
  Plib::systemstate.config.max_tile_id = tileinfo.max_tile_id();

  if ( !Plib::systemstate.config.max_tile_id )
  {
    ERROR_PRINT << "\nError reading tiledata.mul:\n - The file is either corrupted or has an "
                   "unknown format.\n\n";

    throw std::runtime_error( "Unknown format of tiledata.mul" );
  }

  INFO_PRINTLN(
      "Using auto-detected parameters:\n"
      "\tUseNewHSAFormat = {}\n"
      "\tMaxTileID = {:#x}",
      Plib::cfg_use_new_hsa_format, Plib::systemstate.config.max_tile_id );
}

void open_map( void )
{
  size_t map_size;
  // First tries to load the new UOP files. Otherwise fall back to map[N].mul files.
  // map1 uses map0 + 'dif' files, unless there is a map1.mul (newer clients)
  // same for staidx and statics

  if ( !uo_readuop || !open_uopmap_file( uo_mapid, &map_size ) )
    mapfile = open_map_file( "map", uo_mapid, &map_size );

  // Store the file size in a global variable (UGH!)
  uo_map_size = map_size;
}

void open_uo_data_files( void )
{
  open_map();

  sidxfile = open_map_file( "staidx", uo_mapid );
  statfile = open_map_file( "statics", uo_mapid );

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
}  // namespace Plib
}  // namespace Pol
