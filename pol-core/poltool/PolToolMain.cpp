#include "PolToolMain.h"

#include <fstream>
#include <string>

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
#include "baredistro.h"
#include "testenv.h"
#include "testfiles.h"


#include <zlib.h>


namespace Pol::Clib
{
using namespace std;

///////////////////////////////////////////////////////////////////////////////

PolToolMain::PolToolMain() : ProgramMain() {}
///////////////////////////////////////////////////////////////////////////////

void PolToolMain::showHelp()
{
  ERROR_PRINTLN(
      "Usage:\n"
      "    \n"
      "  POLTOOL mapdump [options]\n"
      "        Options:\n"
      "            x1 y1 [x2 y2 realm]       writes polmap info to polmap.html\n"
      "  POLTOOL uncompressgump FileName\n"
      "        unpacks and prints 0xDD gump from given packet log\n"
      "        file needs to contain a single 0xDD packetlog\n"
      "  POLTOOL testfiles [options]\n"
      "        Options:\n"
      "          outdir=.\n"
      "          hsa=0\n"
      "          maxtiles=0x3fff\n"
      "          width=6144\n"
      "          height=4096" );
}

int PolToolMain::mapdump()
{
  short wxl = 5485, wxh = 5500, wyl = 0, wyh = 30;
  const std::vector<std::string>& binArgs = programArgs();
  std::string realmname = "britannia";
  if ( binArgs.size() >= 7 )
  {
    realmname = binArgs[6];
  }
  Plib::RealmDescriptor descriptor = Plib::RealmDescriptor::Load(
      realmname.c_str() );  // TODO: use a string in the signature of Load()
  Plib::MapServer* mapserver = Plib::MapServer::Create( descriptor );
  std::unique_ptr<Plib::MapServer> _owner( mapserver );

  std::unique_ptr<Plib::MapTileServer> mts( new Plib::MapTileServer( descriptor ) );
  if ( binArgs.size() >= 4 )
  {
    wxl = wxh = static_cast<short>( atoi( binArgs[2].c_str() ) );
    wyl = wyh = static_cast<short>( atoi( binArgs[3].c_str() ) );
  }
  if ( binArgs.size() >= 6 )
  {
    wxh = static_cast<short>( atoi( binArgs[4].c_str() ) );
    wyh = static_cast<short>( atoi( binArgs[5].c_str() ) );
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
        ofs << "<br>" << "solid.z=" << int( mlist[i].z ) << "<br>"
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
      if ( per_line == 16 )  // both razor and pol have 16 bytes per line
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
      int num = std::stoi( tmp, nullptr, 16 );
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
  std::string msg = fmt::format( "len {}", len );
  index += 2;
  auto serial = toInt( index );
  index += 4;
  msg += fmt::format( " serial {}", serial );
  auto gumpid = static_cast<unsigned int>( toInt( index ) );
  index += 4;
  msg += fmt::format( " gumpID {}", gumpid );

  auto x = toInt( index );
  index += 4;
  msg += fmt::format( " x {}", x );
  auto y = toInt( index );
  index += 4;
  msg += fmt::format( " y {}", y );
  INFO_PRINTLN( msg );
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
  msg = '\n';
  for ( const auto& c : layout )
  {
    msg += c;
    if ( c == '}' )
      msg += '\n';
  }
  INFO_PRINTLN( msg );

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
  msg = "\n\n";
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
      msg += '"' + u8_conv + "\"\n";
    }
    else
      msg += "\"\"\n";
    j += 2 + wc * 2;
  }
  INFO_PRINTLN( msg );
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
  if ( binArgs[1] == "uncompressgump" )
  {
    return unpackCompressedGump();
  }
  if ( binArgs[1] == "testfiles" )
  {
    std::string outdir = programArgsFindEquals( "outdir=", "." );
    bool hsa = programArgsFindEquals( "hsa=", 0, false ) != 0 ? true : false;
    int maxtiles = programArgsFindEquals( "maxtiles=", 0x3fff, true );
    int width = programArgsFindEquals( "width=", 6144, false );
    int height = programArgsFindEquals( "height=", 4096, false );
    int mapid = programArgsFindEquals( "mapid=", 0, false );
    PolTool::FileGenerator g( outdir, hsa, maxtiles, mapid, width, height );
    g.generateTiledata();
    g.generateMap();
    g.generateStatics();
    g.generateMultis();
    return 0;
  }
  if ( binArgs[1] == "baredistro" )
  {
    std::string outdir = programArgsFindEquals( "outdir=", "." );
    bool hsa = programArgsFindEquals( "hsa=", 0, false ) != 0 ? true : false;
    int maxtiles = programArgsFindEquals( "maxtiles=", 0x3fff, true );
    int width = programArgsFindEquals( "width=", 6144, false );
    int height = programArgsFindEquals( "height=", 4096, false );
    PolTool::BareDistro distro( outdir, hsa, maxtiles, width, height );
    distro.generate();
    return 0;
  }
  else if ( binArgs[1] == "testenv" )
  {
    std::string outdir = programArgsFindEquals( "outdir=", "." );
    bool hsa = programArgsFindEquals( "hsa=", 0, false ) != 0 ? true : false;
    int maxtiles = programArgsFindEquals( "maxtiles=", 0x3fff, true );
    int width = programArgsFindEquals( "width=", 6144, false );
    int height = programArgsFindEquals( "height=", 4096, false );
    PolTool::TestEnv testenv( outdir, hsa, maxtiles, width, height );
    testenv.generate();
    return 0;
  }
  else
  {
    ERROR_PRINTLN( "Unknown command {}", binArgs[1] );
    return 1;  // return "error"
  }
}
}  // namespace Pol::Clib


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{
  Pol::Clib::PolToolMain* PolToolMain = new Pol::Clib::PolToolMain();
  PolToolMain->start( argc, argv );
}
