#include "PolToolMain.h"

#include "../clib/strutil.h"
#include "../clib/logfacility.h"

#include "../plib/mapcell.h"
#include "../plib/mapfunc.h"
#include "../plib/mapserver.h"
#include "../plib/filemapserver.h"
#include "../plib/mapshape.h"
#include "../plib/realmdescriptor.h"
#include "../plib/maptile.h"
#include "../plib/maptileserver.h"

#include <string>
#include <fstream>

namespace Pol
{
namespace Clib
{
using namespace std;

///////////////////////////////////////////////////////////////////////////////

PolToolMain::PolToolMain() : ProgramMain()
{
}
PolToolMain::~PolToolMain()
{
}
///////////////////////////////////////////////////////////////////////////////

void PolToolMain::showHelp()
{
  ERROR_PRINT << "Usage:\n"
              << "    \n"
              << "  POLTOOL mapdump [options]\n"
              << "        Options:\n"
              << "            x1 y1 [x2 y2 realm]       writes polmap info to polmap.html\n";
}

int PolToolMain::poltool()
{
  short wxl = 5485, wxh = 5500, wyl = 0, wyh = 30;
  const std::vector<std::string> binArgs = programArgs();

  std::string realmname = "britannia";
  if ( binArgs.size() >= 6 )
  {
    realmname = binArgs[5];
  }
  Plib::RealmDescriptor descriptor = Plib::RealmDescriptor::Load(
      realmname.c_str() );  // TODO: use a string in the signature of Load()
  Plib::MapServer* mapserver = Plib::MapServer::Create( descriptor );
  std::unique_ptr<Plib::MapServer> _owner( mapserver );

  std::unique_ptr<Plib::MapTileServer> mts( new Plib::MapTileServer( descriptor ) );
  if ( binArgs.size() >= 3 )
  {
    wxl = wxh = static_cast<short>( atoi( binArgs[1].c_str() ) );
    wyl = wyh = static_cast<short>( atoi( binArgs[2].c_str() ) );
  }
  if ( binArgs.size() >= 5 )
  {
    wxh = static_cast<short>( atoi( binArgs[3].c_str() ) );
    wyh = static_cast<short>( atoi( binArgs[4].c_str() ) );
  }

  std::ofstream ofs( "polmap.html" );

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
      mapserver->GetMapShapes( mlist, x, y, static_cast<u32>( Plib::FLAG::ALL ) );
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

int PolToolMain::main()
{
  const std::vector<std::string> binArgs = programArgs();

  /**********************************************
   * show help
   **********************************************/
  if ( binArgs.size() == 1 )
  {
    showHelp();
    return 0;  // return "okay"
  }

  /**********************************************
   * execute the map dumping
   **********************************************/
  if ( binArgs[1] == "mapdump" )
  {
    return poltool();
  }
  else
  {
    ERROR_PRINT << "Unknown command " << binArgs[1] << "\n";
    return 1;  // return "error"
  }
}
}
}  // namespaces

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{
  Pol::Clib::PolToolMain* PolToolMain = new Pol::Clib::PolToolMain();
  PolToolMain->start( argc, argv );
}
