#include "RawMap.h"

#include "../clib/passert.h"

#include "uopreader/uop.h"
#include "uopreader/uophash.h"

#include <cstdio>

using namespace Pol::Core;

namespace Pol {
namespace Plib {

USTRUCT_MAPINFO RawMap::get_cell( unsigned int blockidx, unsigned int x_offset, unsigned int y_offset )
{
  return m_mapinfo_vec.at( blockidx ).cell[y_offset][x_offset];
}

signed char RawMap::rawinfo( unsigned short x, unsigned short y, USTRUCT_MAPINFO * gi )
{
  passert_r( is_init, "Tried to read from map before reading the map files" );
  passert( m_mapwidth > 0 && m_mapheight > 0 && x < m_mapwidth && y < m_mapheight );

  unsigned int x_block = x / 8;
  unsigned int y_block = y / 8;
  unsigned int block = (x_block * (m_mapheight / 8) + y_block);

  unsigned int x_offset = x & 0x7;
  unsigned int y_offset = y & 0x7;

  *gi = get_cell( block, x_offset, y_offset );
  return gi->z;
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

unsigned int RawMap::load_map_difflist( FILE * mapdifl_file )
{
  unsigned index = 0;
  if ( mapdifl_file != NULL )
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

  if ( mapdifl.size() > 0 && mapdif_file == nullptr )
    throw std::runtime_error( "load_full_map: mapdifl is loaded but mapdif is not" );    

  unsigned int block = 0;
  USTRUCT_MAPINFO_BLOCK buffer;

  while ( fread( &buffer, sizeof buffer, 1, mapfile ) == 1 )
  {
    auto citr = mapdifl.find( block );
    if ( citr == mapdifl.end() )
    {
      add_block( buffer );
    }
    else
    {
      // it's in the dif file.. get it from there.
      unsigned dif_index = (*citr).second;
      unsigned int file_offset = dif_index * sizeof( USTRUCT_MAPINFO_BLOCK );
      if ( fseek( mapdif_file, file_offset, SEEK_SET ) != 0 )
        throw std::runtime_error( "rawmapinfo: fseek(mapdif_file) failure" );

      if ( fread( &buffer, sizeof buffer, 1, mapdif_file ) != 1 )
        throw std::runtime_error( "rawmapinfo: fread(mapdif_file) failure" );
      add_block( buffer );
    }
    ++block;
  }

  is_init = true;
  return block;
}

// TODO: cleanup the code
unsigned int RawMap::load_full_map( int uo_mapid, std::istream &ifs )
{
  auto maphash = []( int mapid, int chunkidx ) {
    char mapstring[1024];
    snprintf( mapstring, sizeof mapstring, "build/map%dlegacymul/%08i.dat", mapid, chunkidx );
    return HashLittle2( mapstring );
  };

  if ( !ifs ) {
    ERROR_PRINT << "Error when opening UOP file.\n";
    return 0;
  }

  kaitai::kstream ks( &ifs );
  uop_t uopfile( &ks );

  // TODO: read all blocks (shouldn't be a problem for the current map UOPs though)
  size_t totalSize = 0;
  std::map<uint64_t, uop_t::file_t*> filemap;
  uop_t::block_addr_t *currentblock = uopfile.header()->firstblock();
  for ( auto file : *currentblock->block_body()->files() ) {
    if ( file == nullptr )
      continue;
    if ( file->decompressed_size() == 0 )
      continue;

    filemap[file->filehash()] = file;
    totalSize += file->decompressed_size();
  }

  if ( uopfile.header()->nfiles() != filemap.size() )
    INFO_PRINT << "Warning: not all chunks read (" << filemap.size() << "/" << uopfile.header()->nfiles() << ")\n";

  passert_r( size_t( totalSize / sizeof( USTRUCT_MAPINFO_BLOCK ) ) * sizeof( USTRUCT_MAPINFO_BLOCK ) == totalSize, "Not an integer number of blocks! Check if the sizes are correct.");
  
  m_mapinfo_vec.clear();
  m_mapinfo_vec.resize( totalSize/sizeof( USTRUCT_MAPINFO_BLOCK ) );

  int vecidx = 0;
  for ( size_t i = 0, remaining = totalSize; i < filemap.size(); i++ ) {
    auto fileitr = filemap.find( maphash( uo_mapid, i ) );
    if ( fileitr == filemap.end() ) {
      ERROR_PRINT << "Couldn't find file hash: " << std::to_string(maphash( uo_mapid, i )) << "\n";
      continue;
    }

    auto file = fileitr->second;

    passert( remaining >= file->data()->filebytes().size() );
    memcpy(&m_mapinfo_vec[vecidx], file->data()->filebytes().data(), file->data()->filebytes().size());
    
    remaining -= file->data()->filebytes().size();
    vecidx += file->data()->filebytes().size() / sizeof( USTRUCT_MAPINFO_BLOCK );
  }

  if ( m_mapinfo_vec.size() > 0)
    is_init = true;

  return m_mapinfo_vec.size();
}

RawMap::RawMap() : m_mapwidth(0), m_mapheight(0)
{
}


RawMap::~RawMap()
{
}

}
}