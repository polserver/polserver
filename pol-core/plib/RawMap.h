#pragma once

#include <vector>
#include "../pol/ustruct.h" // Needed because of USTRUCT_MAPINFO_BLOCK

namespace Pol {
  namespace Core {
    class RawMap
    {
    private:
      std::vector<USTRUCT_MAPINFO_BLOCK> m_mapinfo_vec;
      
      unsigned short m_mapwidth;
      unsigned short m_mapheight;

    public:
      // Gets information about a single tile within a 8x8 cell
      USTRUCT_MAPINFO get_cell( unsigned int blockidx, unsigned int x_offset, unsigned int y_offset );
      
      // Returns information about a given coordinate in the world
      signed char mapinfo( unsigned short x, unsigned short y, USTRUCT_MAPINFO* gi );
      
      // Inserts another block of 8x8 tiles
      void add_block( const USTRUCT_MAPINFO_BLOCK& block );

      // Sets map dimensions
      void set_bounds( unsigned int width, unsigned int height );

      RawMap();
      ~RawMap();
    };

  }
}

