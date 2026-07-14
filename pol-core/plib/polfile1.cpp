/** @file
 *
 * @par History
 * - 2005/03/01 Shinigami: extended error message for passert(pstat[i].graphic < 0x4000)
 * - 2005/07/16 Shinigami: added uoconvert.cfg flag ShowIllegalGraphicWarning
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 */


#include <cstdio>
#include <string>
#include <unordered_set>

#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/rawtypes.h"
#include "clidata.h"
#include "realmdescriptor.h"
#include "staticblock.h"
#include "systemstate.h"
#include "uofile.h"
#include "ustruct.h"


namespace Pol::Plib
{
bool cfg_show_illegal_graphic_warning = true;

namespace
{
// (objtype, xy, z, hue) packed into a u64. Height isn't part of the key:
// height is a pure function of objtype (tileheight_read), so objtype equality
// already implies height equality.
bool newstat_dont_add( std::unordered_set<u64>& seen, const USTRUCT_STATIC* pstat )
{
  unsigned char xy = ( pstat->x_offset << 4 ) | pstat->y_offset;
  u64 key = ( static_cast<u64>( pstat->graphic ) << 32 ) | ( static_cast<u64>( xy ) << 24 ) |
            ( static_cast<u64>( static_cast<unsigned char>( pstat->z ) ) << 16 ) | pstat->hue;
  return !seen.insert( key ).second;
}
}  // namespace

int write_pol_static_files( const std::string& realm )
{
#ifndef NDEBUG
  unsigned int duplicates = 0;
  unsigned int illegales = 0;
  unsigned int statics = 0;
  unsigned int empties = 0;
  unsigned int nonempties = 0;
  unsigned int maxcount = 0;
#endif
  std::string directory = "realm/" + realm + "/";

  std::string statidx_dat = directory + "statidx.dat";
  std::string statics_dat = directory + "statics.dat";
  std::string statidx_tmp = directory + "statidx.tmp";
  std::string statics_tmp = directory + "statics.tmp";
  Clib::RemoveFile( statidx_dat );
  Clib::RemoveFile( statics_dat );
  Clib::RemoveFile( statidx_tmp );
  Clib::RemoveFile( statics_tmp );

  FILE* fidx = fopen( statidx_tmp.c_str(), "wb" );
  FILE* fdat = fopen( statics_tmp.c_str(), "wb" );

  RealmDescriptor descriptor = RealmDescriptor::Load( realm );

  int lastprogress = -1;
  unsigned int index = 0;
  std::unordered_set<u64> seen;
  for ( u16 y = 0; y < descriptor.height; y += STATICBLOCK_CHUNK )
  {
    int progress = y * 100L / descriptor.height;
    if ( progress != lastprogress )
    {
      INFO_PRINT( "\rCreating POL statics files: {}%", progress );
      lastprogress = progress;
    }
    for ( u16 x = 0; x < descriptor.width; x += STATICBLOCK_CHUNK )
    {
      STATIC_INDEX idx;
      idx.index = index;
      fwrite( &idx, sizeof idx, 1, fidx );

      const std::vector<USTRUCT_STATIC>& pstat = getstaticblock( x, y );
      std::vector<STATIC_ENTRY> vec;
      seen.clear();
      for ( const auto& stat : pstat )
      {
        if ( stat.graphic <= systemstate.config.max_tile_id )
        {
          if ( !newstat_dont_add( seen, &stat ) )
          {
            STATIC_ENTRY nrec;

            nrec.objtype = stat.graphic;  // TODO map these?
            nrec.xy = ( stat.x_offset << 4 ) | stat.y_offset;
            nrec.z = stat.z;
            nrec.hue = stat.hue;
            vec.push_back( nrec );
#ifndef NDEBUG
            ++statics;
#endif
          }
          else
          {
#ifndef NDEBUG
            ++duplicates;
#endif
          }
        }
        else
        {
#ifndef NDEBUG
          ++illegales;
#endif

          if ( cfg_show_illegal_graphic_warning )
            INFO_PRINTLN( " Warning: Item with illegal Graphic {:#x} in Area {} {} {} {}",
                          stat.graphic, x, y, ( x + STATICBLOCK_CHUNK - 1 ),
                          ( y + STATICBLOCK_CHUNK - 1 ) );
        }
      }
      for ( auto& i : vec )
      {
        fwrite( &i, sizeof( STATIC_ENTRY ), 1, fdat );
        ++index;
      }
#ifndef NDEBUG
      if ( vec.empty() )
        ++empties;
      else
        ++nonempties;
      if ( vec.size() > maxcount )
        maxcount = static_cast<unsigned int>( vec.size() );
#endif
    }
  }
  STATIC_INDEX idx;
  idx.index = index;
  fwrite( &idx, sizeof idx, 1, fidx );

  int errors = ferror( fdat ) || ferror( fidx );
  fclose( fdat );
  fclose( fidx );
  if ( !errors )
  {
    INFO_PRINTLN( "\rCreating POL statics files: Complete" );
    rename( statidx_tmp.c_str(), statidx_dat.c_str() );
    rename( statics_tmp.c_str(), statics_dat.c_str() );
  }
  else
  {
    INFO_PRINTLN( "\rCreating POL statics files: Error" );
  }


#ifndef NDEBUG
  INFO_PRINTLN(
      "{} statics written\n"
      "{} duplicates eliminated\n"
      "{} illegales eliminated\n"
      "{} empties\n"
      "{} nonempties\n"
      "{} was the highest count",
      statics, duplicates, illegales, empties, nonempties, maxcount );
#endif
  return 0;
}
}  // namespace Pol::Plib
