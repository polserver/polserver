/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/strutil.h"

#include "../plib/mapcell.h"
#include "../plib/mapserver.h"
#include "../plib/filemapserver.h"
#include "../plib/mapshape.h"
#include "../plib/realmdescriptor.h"

#include "../plib/realm.h"
#include "../plib/maptile.h"
#include "../plib/maptileserver.h"
namespace Pol {
  namespace Plib {
    void pol_walk_test();
    string flagstr( unsigned int flags );
    string flagdescs();
  }
  namespace Poltool {

	void usage()
	{
	  cerr << "Usage: poltool [cmd] [options]" << endl;
	  cerr << "\t  mapdump x1 y1 [x2 y2 realm]       writes polmap info to polmap.html" << endl;
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

	  ofstream ofs( "polmap.html" );

	  ofs << Plib::flagdescs() << endl;
	  ofs << "<table border=1 cellpadding=5 cellspacing=0>" << endl;
	  ofs << "<tr><td>&nbsp;</td>";
	  for ( int x = wxl; x <= wxh; ++x )
	  {
		ofs << "<td align=center>" << x << "</td>";
	  }
	  ofs << "</tr>" << endl;
	  for ( unsigned short y = wyl; y <= wyh; ++y )
	  {
		ofs << "<tr><td valign=center>" << y << "</td>" << endl;
		for ( unsigned short x = wxl; x <= wxh; ++x )
		{
		  ofs << "<td align=left valign=top>";

		  Plib::MAPCELL cell = mapserver->GetMapCell( x, y );
		  Plib::MapShapeList mlist;
		  mapserver->GetMapShapes( mlist, x, y, Plib::FLAG::ALL );
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

		  ofs << "</td>" << endl;
		}
		ofs << "</tr>" << endl;
	  }
	  ofs << "</table>" << endl;
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

	string cmd = argv[1];
	if ( cmd == "mapdump" )
	{
	  return Poltool::mapdump( argc - 1, argv + 1 );
	}
	else
	{
	  cerr << "Unknown command " << cmd << endl;
	  return 1;
	}
  }
}