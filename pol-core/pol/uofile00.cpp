/** @file
 *
 * @par History
 */

#include "uofile.h"

#include <cstdio>
#include <string>

#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/stlutil.h"
#include "../plib/systemstate.h"

#ifdef _MSC_VER
#pragma warning( disable : 4996 )  // disable deprecation warning for fopen, etc
#endif

namespace Pol
{
namespace Core
{
FILE* mapfile;
FILE* sidxfile;
FILE* statfile;
FILE* verfile;
FILE* tilefile;
FILE* stadifl_file;
FILE* stadifi_file;
FILE* stadif_file;
FILE* mapdifl_file;
FILE* mapdif_file;

FILE* open_uo_file( const std::string& filename_part )
{
  std::string filename = Plib::systemstate.config.uo_datafile_root + filename_part;
  FILE* fp = fopen( filename.c_str(), "rb" );
  if ( !fp )
  {
    ERROR_PRINT << "Unable to open UO datafile: " << filename << "\n"
                << "POL.CFG specifies UODataFileRoot as '"
                << Plib::systemstate.config.uo_datafile_root << "'.  Is this correct?\n"
                << "  The following files must be present in that directory:\n"
                << "      map0.mul\n"
                << "      multi.idx\n"
                << "      multi.mul\n"
                << "      staidx0.mul\n"
                << "      statics0.mul\n"
                << "      tiledata.mul\n"
                << "      verdata.mul    (optional - only if present on client install)\n";

    throw std::runtime_error( "Error opening UO datafile." );
  }
  return fp;
}

FILE* open_map_file( std::string name, int map_id )
{
  std::string filename;

  filename = name + Clib::tostring( map_id ) + ".mul";
  if ( uo_mapid == 1 && !Clib::FileExists( Plib::systemstate.config.uo_datafile_root + filename ) )
  {
    ERROR_PRINT << "Unable to find UO file: " << filename
                << ", reading " + name + "0.mul instead.\n";
    filename = name + "0.mul";
  }

  return open_uo_file( filename );
}

int uo_mapid = 0;
int uo_usedif = 0;
unsigned short uo_map_width = 6144;
unsigned short uo_map_height = 4096;
void open_uo_data_files( void )
{
  std::string filename;
  // map1 uses map0 + 'dif' files, unless there is a map1.mul (newer clients)
  // same for staidx and statics
  mapfile = open_map_file( "map", uo_mapid );
  sidxfile = open_map_file( "staidx", uo_mapid );
  statfile = open_map_file( "statics", uo_mapid );

  if ( Clib::FileExists( ( Plib::systemstate.config.uo_datafile_root + "verdata.mul" ).c_str() ) )
  {
    verfile = open_uo_file( "verdata.mul" );
  }
  else
  {
    verfile = NULL;
  }
  tilefile = open_uo_file( "tiledata.mul" );

  if ( uo_usedif )
  {
    filename = "stadifl" + Clib::tostring( uo_mapid ) + ".mul";
    if ( Clib::FileExists( Plib::systemstate.config.uo_datafile_root + filename ) )
    {
      stadifl_file = open_uo_file( filename );
      filename = "stadifi" + Clib::tostring( uo_mapid ) + ".mul";
      stadifi_file = open_uo_file( filename );
      filename = "stadif" + Clib::tostring( uo_mapid ) + ".mul";
      stadif_file = open_uo_file( filename );
    }
    filename = "mapdifl" + Clib::tostring( uo_mapid ) + ".mul";
    if ( Clib::FileExists( Plib::systemstate.config.uo_datafile_root + filename ) )
    {
      mapdifl_file = open_uo_file( filename );
      filename = "mapdif" + Clib::tostring( uo_mapid ) + ".mul";
      mapdif_file = open_uo_file( filename );
    }
  }
}
}
}
