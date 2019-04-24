/** @file
 *
 * @par History
 * - 2005/03/01 Shinigami: extended error message for passert(pstat[i].graphic < 0x4000)
 * - 2005/07/16 Shinigami: added uoconvert.cfg flag ShowIllegalGraphicWarning
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 */


#include <cstdio>
#include <string>

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

namespace Pol
{
namespace Plib
{
bool cfg_show_illegal_graphic_warning = true;

bool newstat_dont_add( std::vector<STATIC_ENTRY>& vec, USTRUCT_STATIC* pstat )
{
  char pheight = tileheight_read( pstat->graphic );

  for ( unsigned i = 0; i < vec.size(); ++i )
  {
    STATIC_ENTRY& prec = vec[i];
    passert_always( prec.objtype <= systemstate.config.max_tile_id );
    char height = tileheight_read( prec.objtype );  // TODO read from itemdesc?
    unsigned char xy = ( pstat->x_offset << 4 ) | pstat->y_offset;
    if (                                   // flags == pflags &&
        prec.objtype == pstat->graphic &&  // TODO map objtype->graphic from itemdesc
        height == pheight && prec.xy == xy && prec.z == pstat->z && prec.hue == pstat->hue )
    {
      return true;
    }
  }
  return false;
}

int write_pol_static_files( const std::string& realm )
{
  unsigned int duplicates = 0;
  unsigned int illegales = 0;
  unsigned int statics = 0;
  unsigned int empties = 0;
  unsigned int nonempties = 0;
  unsigned int maxcount = 0;

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
  for ( u16 y = 0; y < descriptor.height; y += STATICBLOCK_CHUNK )
  {
    int progress = y * 100L / descriptor.height;
    if ( progress != lastprogress )
    {
      INFO_PRINT << "\rCreating POL statics files: " << progress << "%";
      lastprogress = progress;
    }
    for ( u16 x = 0; x < descriptor.width; x += STATICBLOCK_CHUNK )
    {
      STATIC_INDEX idx;
      idx.index = index;
      fwrite( &idx, sizeof idx, 1, fidx );

      std::vector<USTRUCT_STATIC> pstat;
      int num;
      std::vector<STATIC_ENTRY> vec;
      readstaticblock( &pstat, &num, x, y );
      for ( int i = 0; i < num; ++i )
      {
        if ( pstat[i].graphic <= systemstate.config.max_tile_id )
        {
          if ( !newstat_dont_add( vec, &pstat[i] ) )
          {
            STATIC_ENTRY nrec;

            nrec.objtype = pstat[i].graphic;  // TODO map these?
            nrec.xy = ( pstat[i].x_offset << 4 ) | pstat[i].y_offset;
            nrec.z = pstat[i].z;
            nrec.hue = pstat[i].hue;
            vec.push_back( nrec );
            ++statics;
          }
          else
          {
            ++duplicates;
          }
        }
        else
        {
          ++illegales;

          if ( cfg_show_illegal_graphic_warning )
            INFO_PRINT << " Warning: Item with illegal Graphic 0x" << fmt::hexu( pstat[i].graphic )
                       << " in Area " << x << " " << y << " " << ( x + STATICBLOCK_CHUNK - 1 )
                       << " " << ( y + STATICBLOCK_CHUNK - 1 ) << "\n";
        }
      }
      for ( unsigned i = 0; i < vec.size(); ++i )
      {
        fwrite( &vec[i], sizeof( STATIC_ENTRY ), 1, fdat );
        ++index;
      }
      if ( vec.empty() )
        ++empties;
      else
        ++nonempties;
      if ( vec.size() > maxcount )
        maxcount = static_cast<unsigned int>( vec.size() );
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
    INFO_PRINT << "\rCreating POL statics files: Complete\n";
    rename( statidx_tmp.c_str(), statidx_dat.c_str() );
    rename( statics_tmp.c_str(), statics_dat.c_str() );
  }
  else
  {
    INFO_PRINT << "\rCreating POL statics files: Error\n";
  }


#ifndef NDEBUG
  INFO_PRINT << statics << " statics written\n"
             << duplicates << " duplicates eliminated\n"
             << illegales << " illegales eliminated\n"
             << empties << " empties\n"
             << nonempties << " nonempties\n"
             << maxcount << " was the highest count\n";
#endif
  return 0;
}
}  // namespace Plib
}  // namespace Pol
