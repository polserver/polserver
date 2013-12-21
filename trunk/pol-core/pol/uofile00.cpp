/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#include <stdio.h>
#include <string.h>

#include "../clib/fileutil.h"
#include "../clib/stlutil.h"

#include "polcfg.h"
#include "polfile.h"
#include "udatfile.h"
#include "ustruct.h"

#include "uofile.h"

namespace Pol {
  namespace Core {
    FILE *mapfile;
    FILE *sidxfile;
    FILE *statfile;
    FILE *verfile;
    FILE *tilefile;
    FILE *stadifl_file;
    FILE *stadifi_file;
    FILE *stadif_file;
    FILE *mapdifl_file;
    FILE *mapdif_file;

    FILE *open_uo_file( const string& filename_part )
    {
      string filename = config.uo_datafile_root + filename_part;
      FILE *fp = fopen( filename.c_str(), "rb" );
      if ( !fp )
      {
        cerr << "Unable to open UO datafile: " << filename << endl;
        cerr << "POL.CFG specifies UODataFileRoot as '" << config.uo_datafile_root << "'.  Is this correct?" << endl;
        cerr << "  The following files must be present in that directory:" << endl
          << "      map0.mul" << endl
          << "      multi.idx" << endl
          << "      multi.mul" << endl
          << "      staidx0.mul" << endl
          << "      statics0.mul" << endl
          << "      tiledata.mul" << endl
          << "      verdata.mul    (optional - only if present on client install)" << endl;

        throw runtime_error( "Error opening UO datafile." );
      }
      return fp;
    }

    FILE* open_map_file( string name, int map_id )
    {
      string filename;

      filename = name + Clib::tostring( map_id ) + ".mul";
      if ( uo_mapid == 1 && !Clib::FileExists( config.uo_datafile_root + filename ) )
      {
        cerr << "Unable to find UO file: " << filename << ", reading " + name + "0.mul instead." << endl;
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
      string filename;
      // map1 uses map0 + 'dif' files, unless there is a map1.mul (newer clients)
      // same for staidx and statics
      mapfile = open_map_file( "map", uo_mapid );
      sidxfile = open_map_file( "staidx", uo_mapid );
      statfile = open_map_file( "statics", uo_mapid );

      if ( Clib::FileExists( ( config.uo_datafile_root + "verdata.mul" ).c_str( ) ) )
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
        if ( Clib::FileExists( config.uo_datafile_root + filename ) )
        {
          stadifl_file = open_uo_file( filename );
          filename = "stadifi" + Clib::tostring( uo_mapid ) + ".mul";
          stadifi_file = open_uo_file( filename );
          filename = "stadif" + Clib::tostring( uo_mapid ) + ".mul";
          stadif_file = open_uo_file( filename );
        }
        filename = "mapdifl" + Clib::tostring( uo_mapid ) + ".mul";
        if ( Clib::FileExists( config.uo_datafile_root + filename ) )
        {
          mapdifl_file = open_uo_file( filename );
          filename = "mapdif" + Clib::tostring( uo_mapid ) + ".mul";
          mapdif_file = open_uo_file( filename );
        }
      }
    }
  }
}
