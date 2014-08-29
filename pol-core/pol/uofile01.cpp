/*
History
=======
2009/12/02 Turley:    added config.max_tile_id - Tomi

Notes
=======

*/

#include "../clib/stl_inc.h"

#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif


#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../clib/passert.h"

#include "../plib/mapfunc.h"

#include "objtype.h"
#include "polcfg.h"
#include "polfile.h"
#include "profile.h"
#include "udatfile.h"
#include "ustruct.h"
#include "wrldsize.h"

#include "uofile.h"
#include "uofilei.h"
namespace Pol {
  namespace Core {
#define VERFILE_TILEDATA 0x1E
#define TILEDATA_TILES 0x68800
#define TILEDATA_TILES_HSA 0x78800

    struct TileData
    {
      u8 height;
      u8 layer;
      u32 flags;
    };
    TileData *tiledata;

    const unsigned N_LANDTILEDATA = 0x4000;
    unsigned int landtile_flags_arr[N_LANDTILEDATA];

    unsigned int landtile_uoflags( unsigned short landtile )
    {
      passert_always( landtile < N_LANDTILEDATA );
      return landtile_flags_arr[landtile];
    }


    struct VerdataIndexes
    {
      typedef map<unsigned int, Core::USTRUCT_VERSION> VRecList;
      VRecList vrecs; // key is the block

      void insert( const Core::USTRUCT_VERSION& vrec );
      bool find( unsigned int block, const Core::USTRUCT_VERSION*& vrec );
    };
    void VerdataIndexes::insert( const Core::USTRUCT_VERSION& vrec )
    {
      vrecs.insert( VRecList::value_type( vrec.block, vrec ) );
    }
    bool VerdataIndexes::find( unsigned int block, const Core::USTRUCT_VERSION*& vrec )
    {
      VRecList::const_iterator itr = vrecs.find( block );
      if ( itr == vrecs.end() )
        return false;

      vrec = &( ( *itr ).second );
      return true;

    }


    VerdataIndexes vidx[32];
    const unsigned int vidx_count = 32;

    bool check_verdata( unsigned int file, unsigned int block, const Core::USTRUCT_VERSION*& vrec )
    {
      return vidx[file].find( block, vrec );
    }

    static bool seekto_newer_version( unsigned int file, unsigned int block )
    {
      const Core::USTRUCT_VERSION* vrec;
      if ( vidx[file].find( block, vrec ) )
      {
        fseek( Core::verfile, vrec->filepos, SEEK_SET );
        return true;
      }
      else
      {
        return false;
      }

    }

    void readtile( unsigned short tilenum, Core::USTRUCT_TILE *tile )
    {
      if ( tilenum > Core::config.max_tile_id )
      {
        memset( tile, 0, sizeof *tile );
        sprintf( tile->name, "multi" );
        tile->weight = 0xFF;
      }
      else
      {
        int block;
        block = ( tilenum / 32 );
        if ( seekto_newer_version( VERFILE_TILEDATA, block + 0x200 ) )
        {
          int filepos;
          filepos = 4 +
            ( sizeof *tile ) * ( tilenum & 0x1F );
          fseek( Core::verfile, filepos, SEEK_CUR );
          fread( tile, sizeof *tile, 1, Core::verfile );
        }
        else
        {
          //512 blocks
          // header 4
          // 32*size (30)
          int filepos;
          filepos = TILEDATA_TILES +
            ( block * 4 ) +         // skip headers of all previous blocks
            4 +                   // skip my header
            ( sizeof( Core::USTRUCT_TILE )*tilenum );
          fseek( Core::tilefile, filepos, SEEK_SET );
          fread( tile, sizeof *tile, 1, Core::tilefile );
        }
      }

      // ensure string is null-terminated
      tile->name[sizeof(tile->name) - 1] = '\0';
    }

    void readtile( unsigned short tilenum, Core::USTRUCT_TILE_HSA *tile )
    {
      if ( tilenum > Core::config.max_tile_id )
      {
        memset( tile, 0, sizeof *tile );
        sprintf( tile->name, "multi" );
        tile->weight = 0xFF;
      }
      else
      {
        int block;
        block = ( tilenum / 32 );
        if ( seekto_newer_version( VERFILE_TILEDATA, block + 0x200 ) )
        {
          int filepos;
          filepos = 4 +
            ( sizeof *tile ) * ( tilenum & 0x1F );
          fseek( Core::verfile, filepos, SEEK_CUR );
          fread( tile, sizeof *tile, 1, Core::verfile );
        }
        else
        {
          int filepos;
          filepos = TILEDATA_TILES_HSA +
            ( block * 4 ) +         // skip headers of all previous blocks
            4 +                   // skip my header
            ( sizeof( Core::USTRUCT_TILE_HSA )*tilenum );
          fseek( Core::tilefile, filepos, SEEK_SET );
          fread( tile, sizeof *tile, 1, Core::tilefile );
        }
      }

      // ensure string is null-terminated
      tile->name[sizeof(tile->name) - 1] = '\0';
    }


    void readlandtile( unsigned short tilenum, Core::USTRUCT_LAND_TILE* landtile )
    {
      if ( tilenum <= 0x3FFF )
      {
        int block;
        block = ( tilenum / 32 );
        if ( seekto_newer_version( VERFILE_TILEDATA, block ) )
        {
          int filepos;
          filepos = 4 + ( sizeof *landtile ) * ( tilenum & 0x1F );
          fseek( Core::verfile, filepos, SEEK_CUR );
          fread( landtile, sizeof *landtile, 1, Core::verfile );
        }
        else
        {
          int filepos;
          filepos = ( block * 4 ) +         // skip headers of all previous blocks
            4 +                   // skip my header
            ( sizeof( Core::USTRUCT_LAND_TILE )* tilenum );
          fseek( Core::tilefile, filepos, SEEK_SET );
          fread( landtile, sizeof *landtile, 1, Core::tilefile );
        }
      }

      // ensure string is null-terminated
      landtile->name[sizeof(landtile->name) - 1] = '\0';
    }

    void readlandtile( unsigned short tilenum, Core::USTRUCT_LAND_TILE_HSA* landtile )
    {
      if ( tilenum <= 0x3FFF )
      {
        int block;
        block = ( tilenum / 32 );
        if ( seekto_newer_version( VERFILE_TILEDATA, block ) )
        {
          int filepos;
          filepos = 4 + ( sizeof *landtile ) * ( tilenum & 0x1F );
          fseek( Core::verfile, filepos, SEEK_CUR );
          fread( landtile, sizeof *landtile, 1, Core::verfile );
        }
        else
        {
          int filepos;
          filepos = ( block * 4 ) +         // skip headers of all previous blocks
            4 +                   // skip my header
            ( sizeof( Core::USTRUCT_LAND_TILE_HSA )* tilenum );
          fseek( Core::tilefile, filepos, SEEK_SET );
          fread( landtile, sizeof *landtile, 1, Core::tilefile );
        }
      }
      
      // ensure string is null-terminated
      landtile->name[sizeof(landtile->name) - 1] = '\0'; 
    }

    void read_objinfo( u16 graphic, Core::USTRUCT_TILE& objinfo )
    {
      readtile( graphic, &objinfo );
    }

    void read_objinfo( u16 graphic, Core::USTRUCT_TILE_HSA& objinfo )
    {
      readtile( graphic, &objinfo );
    }

    char tileheight( unsigned short tilenum )
    {
      u8 height;
      u32 flags;

      if ( tilenum <= Core::config.max_tile_id )
      {
        height = tiledata[tilenum].height;
        flags = tiledata[tilenum].flags;
      }
      else
      {
        if ( Core::cfg_use_new_hsa_format )
        {
          Core::USTRUCT_TILE_HSA tile;
          readtile( tilenum, &tile );
          height = tile.height;
          flags = tile.flags;
        }
        else
        {
          Core::USTRUCT_TILE tile;
          readtile( tilenum, &tile );
          height = tile.height;
          flags = tile.flags;
        }
      }

      if ( flags & Core::USTRUCT_TILE::FLAG_HALF_HEIGHT )
        return ( height / 2 );
      else
        return height;
    }

    unsigned char tilelayer( unsigned short tilenum )
    {
      if ( tilenum <= Core::config.max_tile_id )
      {
        return tiledata[tilenum].layer;
      }
      else
      {
        if ( Core::cfg_use_new_hsa_format )
        {
          Core::USTRUCT_TILE_HSA tile;
          tile.layer = 0;
          readtile( tilenum, &tile );
          return tile.layer;
        }
        else
        {
          Core::USTRUCT_TILE tile;
          tile.layer = 0;
          readtile( tilenum, &tile );
          return tile.layer;
        }
      }
    }

    u16 tileweight( unsigned short tilenum )
    {
      if ( Core::cfg_use_new_hsa_format )
      {
        Core::USTRUCT_TILE_HSA tile;
        tile.weight = 1;
        readtile( tilenum, &tile );
        return tile.weight;
      }
      else
      {
        Core::USTRUCT_TILE tile;
        tile.weight = 1;
        readtile( tilenum, &tile );
        return tile.weight;
      }
    }

    u32 tile_uoflags( unsigned short tilenum )
    {
      if ( tilenum <= Core::config.max_tile_id )
      {
        return tiledata[tilenum].flags;
      }
      else
      {
        if ( Core::cfg_use_new_hsa_format )
        {
          Core::USTRUCT_TILE_HSA tile;
          tile.flags = 0;
          readtile( tilenum, &tile );
          return tile.flags;
        }
        else
        {
          Core::USTRUCT_TILE tile;
          tile.flags = 0;
          readtile( tilenum, &tile );
          return tile.flags;
        }
      }
    }


    static void read_veridx()
    {
      int num_version_records;
      Core::USTRUCT_VERSION vrec;

      if ( Core::verfile != NULL )
      {
        // FIXME: should read this once per run, per file.
        fseek( verfile, 0, SEEK_SET );
        fread( &num_version_records, sizeof num_version_records, 1, verfile ); // ENDIAN-BROKEN

        for ( int i = 0; i < num_version_records; i++ )
        {
          fread( &vrec, sizeof vrec, 1, verfile );

          if ( vrec.file < vidx_count )
          {
            vidx[vrec.file].insert( vrec );
          }

        }
      }
    }

    static void read_tiledata()
    {
      tiledata = new TileData[Core::config.max_tile_id + 1];

      for ( u32 graphic_i = 0; graphic_i <= config.max_tile_id; ++graphic_i )
      {
        u16 graphic = static_cast<u16>( graphic_i );
        if ( Core::cfg_use_new_hsa_format )
        {
          Core::USTRUCT_TILE_HSA objinfo;
          memset( &objinfo, 0, sizeof objinfo );
          readtile( (u16)graphic, &objinfo );

          tiledata[graphic].height = objinfo.height;
          tiledata[graphic].layer = objinfo.layer;
          tiledata[graphic].flags = objinfo.flags;
        }
        else
        {
          Core::USTRUCT_TILE objinfo;
          memset( &objinfo, 0, sizeof objinfo );
          readtile( graphic, &objinfo );

          tiledata[graphic].height = objinfo.height;
          tiledata[graphic].layer = objinfo.layer;
          tiledata[graphic].flags = objinfo.flags;
        }
      }
    }

    void clear_tiledata()
    {
      delete[] tiledata;
    }

    static void read_landtiledata()
    {
      for ( u16 objtype = 0; objtype < N_LANDTILEDATA; ++objtype )
      {
        if ( Core::cfg_use_new_hsa_format )
        {
          Core::USTRUCT_LAND_TILE_HSA landtile;
          readlandtile( objtype, &landtile );

          landtile_flags_arr[objtype] = landtile.flags;
        }
        else
        {
          Core::USTRUCT_LAND_TILE landtile;
          readlandtile( objtype, &landtile );

          landtile_flags_arr[objtype] = landtile.flags;
        }
      }
    }
    void read_uo_data( void )
    {
      read_veridx();
      read_tiledata();
      read_landtiledata();
      Core::read_static_diffs();
      Core::read_map_difs();
    }
  }
}
