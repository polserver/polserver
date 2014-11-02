/*
History
=======

Notes
=======

*/

#include "../clib/strutil.h"
#include "../clib/logfacility.h"

#include "../plib/mapcell.h"
#include "../plib/mapserver.h"
#include "../plib/filemapserver.h"
#include "../plib/mapshape.h"
#include "../plib/realmdescriptor.h"

#include "../plib/realm.h"
#include "../plib/maptile.h"
#include "../plib/maptileserver.h"

#include <string>
#include <fstream>

namespace Pol {
  namespace Plib {
    void pol_walk_test();
    std::string flagstr( unsigned int flags );
    std::string flagdescs();
  }
  namespace Poltool {

	void usage()
	{
      ERROR_PRINT << "Usage: poltool [cmd] [options]\n"
        << "\t  mapdump x1 y1 [x2 y2 realm]       writes polmap info to polmap.html\n";
	}

	int mapdump( int argc, char* argv[] )
	{
	  short wxl = 5485, wxh = 5500, wyl = 0, wyh = 30;

	  char* realmname = "britannia";
	  if ( argc >= 6 )
	  {
		realmname = argv[5];
	  }
	  Plib::RealmDescriptor descriptor = Plib::RealmDescriptor::Load( realmname );
	  Plib::MapServer* mapserver = Plib::MapServer::Create( descriptor );
	  std::unique_ptr<Plib::MapServer> _owner( mapserver );

	  std::unique_ptr<Plib::MapTileServer> mts( new Plib::MapTileServer( descriptor ) );
	  if ( argc >= 3 )
	  {
		wxl = wxh = static_cast<short>( atoi( argv[1] ) );
		wyl = wyh = static_cast<short>( atoi( argv[2] ) );
	  }
	  if ( argc >= 5 )
	  {
		wxh = static_cast<short>( atoi( argv[3] ) );
		wyh = static_cast<short>( atoi( argv[4] ) );
	  }

      std::ofstream ofs("polmap.html");

      ofs << Plib::flagdescs() << std::endl;
      ofs << "<table border=1 cellpadding=5 cellspacing=0>" << std::endl;
	  ofs << "<tr><td>&nbsp;</td>";
	  for ( int x = wxl; x <= wxh; ++x )
	  {
		ofs << "<td align=center>" << x << "</td>";
	  }
      ofs << "</tr>" << std::endl;
	  for ( unsigned short y = wyl; y <= wyh; ++y )
	  {
          ofs << "<tr><td valign=center>" << y << "</td>" << std::endl;
		for ( unsigned short x = wxl; x <= wxh; ++x )
		{
		  ofs << "<td align=left valign=top>";

		  Plib::MAPCELL cell = mapserver->GetMapCell( x, y );
		  Plib::MapShapeList mlist;
		  mapserver->GetMapShapes( mlist, x, y, static_cast<u32>(Plib::FLAG::ALL) );
		  Plib::MAPTILE_CELL tile = mts->GetMapTile( x, y );

		  ofs << "landtile=" << int( tile.landtile ) << "<br>";
		  ofs << "tilez=" << int( tile.z ) << "<br>";
		  ofs << "z=" << int( cell.z ) << "<br>";
          ofs << "flags=" << Plib::flagstr( cell.flags );

		  for ( unsigned i = 1; i < mlist.size(); ++i )
		  {
			ofs << "<br>"
			  << "solid.z=" << int( mlist[i].z ) << "<br>"
			  << "solid.height=" << int( mlist[i].height ) << "<br>"
              << "solid.flags=" << Plib::flagstr( mlist[i].flags );
		  }

          ofs << "</td>" << std::endl;
		}
        ofs << "</tr>" << std::endl;
	  }
      ofs << "</table>" << std::endl;
	  return 0;
	}
  }

  int xmain( int argc, char* argv[] )
  {
	if ( argc == 1 )
	{
	  Poltool::usage();
	  return 1;
	}

    std::string cmd = argv[1];
	if ( cmd == "mapdump" )
	{
	  return Poltool::mapdump( argc - 1, argv + 1 );
	}
	else
	{
      ERROR_PRINT << "Unknown command " << cmd << "\n";
	  return 1;
	}
  }
}