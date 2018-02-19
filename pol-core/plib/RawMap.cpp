#include "stdafx.h"
#include "RawMap.h"

#include "../clib/passert.h"

using namespace Pol::Core;


USTRUCT_MAPINFO RawMap::get_cell( unsigned int blockidx, unsigned int x_offset, unsigned int y_offset )
{
  return m_mapinfo_vec.at( blockidx ).cell[y_offset][x_offset];
}

signed char Pol::Core::RawMap::mapinfo( unsigned short x, unsigned short y, USTRUCT_MAPINFO * gi )
{
  passert( m_mapwidth > 0 && m_mapheight > 0 && x < m_mapwidth && y < m_mapheight );

  unsigned int x_block = x / 8;
  unsigned int y_block = y / 8;
  unsigned int block = (x_block * (m_mapheight / 8) + y_block);

  unsigned int x_offset = x & 0x7;
  unsigned int y_offset = y & 0x7;

  *gi = get_cell( block, x_offset, y_offset );
  return gi->z;
}

void Pol::Core::RawMap::add_block( const USTRUCT_MAPINFO_BLOCK& block )
{
  m_mapinfo_vec.push_back( block );
}

void Pol::Core::RawMap::set_bounds( unsigned int width, unsigned int height )
{
  m_mapwidth = width;
  m_mapheight = height;
}

RawMap::RawMap() : m_mapwidth(0), m_mapheight(0)
{
}


RawMap::~RawMap()
{
}
