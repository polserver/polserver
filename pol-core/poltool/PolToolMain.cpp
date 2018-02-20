#include "PolToolMain.h"

#include <fstream>
#include <string>

#include <format/format.h>
#include "../clib/Program/ProgramMain.h"
#include "../clib/clib_endian.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/rawtypes.h"
#include "../plib/mapcell.h"
#include "../plib/mapfunc.h"
#include "../plib/mapserver.h"
#include "../plib/mapshape.h"
#include "../plib/maptile.h"
#include "../plib/maptileserver.h"
#include "../plib/realmdescriptor.h"
#ifdef USE_SYSTEM_ZLIB
#include <zlib.h>
#else
#include "../../lib/zlib/zlib.h"
#endif

namespace Pol
{
namespace Clib
{
using namespace std;

///////////////////////////////////////////////////////////////////////////////

PolToolMain::PolToolMain() : ProgramMain() {}
PolToolMain::~PolToolMain() {}
///////////////////////////////////////////////////////////////////////////////

void PolToolMain::showHelp()
{
  ERROR_PRINT << "Usage:\n"
              << "    \n"
              << "  POLTOOL mapdump [options]\n"
              << "        Options:\n"
              << "            x1 y1 [x2 y2 realm]       writes polmap info to polmap.html\n"
              << "  POLTOOL uncompressgump FileName\n"
              << "        unpacks and prints 0xDD gump from given packet log\n"
              << "        file needs to contain a single 0xDD packetlog\n";
}

int PolToolMain::mapdump()
{
  short wxl = 5485, wxh = 5500, wyl = 0, wyh = 30;
  const std::vector<std::string>& binArgs = programArgs();

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

int PolToolMain::unpackCompressedGump()
{
  const std::vector<std::string>& binArgs = programArgs();
  if ( binArgs.size() < 3 )
  {
    showHelp();
    return 1;
  }
  if ( !Clib::FileExists( binArgs[2] ) )
    return 1;

  ifstream dmp( binArgs[2] );
  std::vector<unsigned char> bytes;
  dmp.setf( std::ios::hex );
  int per_line = 0;
  while ( dmp )
  {
    std::string tmp;
    while ( dmp )
    {
      char c;
      dmp.get( c );
      if ( c == ' ' )
      {
        break;
      }
      else if ( per_line == 16 )  // both razor and pol have 16 bytes per line
      {
        if ( c == '\n' )
          per_line = 0;
      }
      else if ( isdigit( c ) )
        tmp += c;
      else if ( c >= 'A' && c <= 'Z' )
        tmp += c;
    }
    if ( tmp.size() == 2 )
    {
      int num = std::stoi( tmp, 0, 16 );
      bytes.push_back( static_cast<unsigned char>( num ) );
      ++per_line;
    }
  }

  size_t index = 0;
  auto toInt = [&]( size_t i ) -> unsigned int {
    return ( bytes[i] << 24 ) | ( bytes[i + 1] << 16 ) | ( bytes[i + 2] << 8 ) | ( bytes[i + 3] );
  };
  auto toShort = [&]( size_t i ) -> unsigned short { return ( bytes[i] << 8 ) | ( bytes[i + 1] ); };
  if ( bytes[index] != 0xdd )
    return 1;
  ++index;
  auto len = toShort( index );
  INFO_PRINT << "len " << len;
  index += 2;
  auto serial = toInt( index );
  index += 4;
  INFO_PRINT << " serial " << serial;
  auto gumpid = static_cast<unsigned int>( toInt( index ) );
  index += 4;
  INFO_PRINT << " gumpID " << gumpid;

  auto x = toInt( index );
  index += 4;
  INFO_PRINT << " x " << x;
  auto y = toInt( index );
  index += 4;
  INFO_PRINT << " y " << y;
  auto cbuflen = toInt( index ) - 4;
  index += 4;
  auto datalen = static_cast<unsigned long>( toInt( index ) );
  index += 4;
  std::unique_ptr<unsigned char[]> uncompressed( new unsigned char[datalen] );
  int res = uncompress( uncompressed.get(), &datalen, &bytes.data()[index], cbuflen );
  if ( res < 0 )
    return 1;
  index += cbuflen;
  std::string layout( uncompressed.get(), uncompressed.get() + datalen - 1 );
  fmt::Writer layouttmp;
  layouttmp << '\n';
  for ( const auto& c : layout )
  {
    layouttmp << c;
    if ( c == '}' )
      layouttmp << '\n';
  }
  INFO_PRINT << layouttmp.str();

  auto linecount = toInt( index );
  index += 4;
  cbuflen = toInt( index ) - 4;
  index += 4;
  datalen = static_cast<unsigned long>( toInt( index ) );
  index += 4;
  std::unique_ptr<unsigned char[]> uncompressed2( new unsigned char[datalen] );
  res = uncompress( uncompressed2.get(), &datalen, &bytes.data()[index], cbuflen );
  if ( res < 0 )
    return 1;
  std::string layout2( uncompressed2.get(), uncompressed2.get() + datalen );
  INFO_PRINT << '\n';
  fmt::Writer datatmp;
  datatmp << '\n';
  size_t j = 0;
  for ( size_t i = 0; i < linecount; ++i )
  {
    unsigned short wc = ( uncompressed2[j] << 8 ) | ( uncompressed2[j + 1] );
    if ( wc > 0 )
    {
      std::u16string u16s( reinterpret_cast<const char16_t*>( &uncompressed2[j + 2] ), wc );
      std::string u8_conv;
      for ( auto c : u16s )
      {
        c = ctBEu16( c );
        if ( c <= 256 )
          u8_conv += static_cast<char>( c );
        else
        {
          u8_conv += static_cast<char>( c & 0xff );
          u8_conv += static_cast<char>( ( c >> 8 ) & 0xff );
        }
      }
      datatmp << '"' << u8_conv << "\"\n";
    }
    else
      datatmp << "\"\"\n";
    j += 2 + wc * 2;
  }
  INFO_PRINT << datatmp.str();
  INFO_PRINT << '\n';
  return 0;
}

int PolToolMain::main()
{
  const std::vector<std::string>& binArgs = programArgs();

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
    return mapdump();
  }
  else if ( binArgs[1] == "uncompressgump" )
  {
    return unpackCompressedGump();
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
