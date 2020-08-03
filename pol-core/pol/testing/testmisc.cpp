/** @file
 *
 * @par History
 */

#include <algorithm>
#include <array>
#include <cstring>
#include <string>

#include "../../clib/logfacility.h"
#include "../../clib/rawtypes.h"
#include "../../plib/maptile.h"
#include "../dynproperties.h"
#include "../globals/uvars.h"
#include "../network/packethelper.h"
#include "../realms/realm.h"

namespace Pol
{
namespace Testing
{
void dummy() {}

void map_test()
{
  Plib::MAPTILE_CELL cell = Core::gamestate.main_realm->getmaptile( 1453, 1794 );
  INFO_PRINT << cell.landtile << " " << cell.z << "\n";
}

void dynprops_test()
{
  class Test : public Core::DynamicPropsHolder
  {
  public:
    DYN_PROPERTY( armod, s16, Core::PROP_AR_MOD, 0 );
    DYN_PROPERTY( max_items, u32, Core::PROP_MAX_ITEMS_MOD, 0 );
    DYN_PROPERTY( itemname, std::string, Core::PROP_NAME_SUFFIX, "" );
  };
  Test h;
  INFO_PRINT << "size " << h.estimateSizeDynProps() << "\n";
  INFO_PRINT << "ar " << h.armod() << " " << h.has_armod() << "\n";
  h.armod( 10 );
  INFO_PRINT << "ar " << h.armod() << " " << h.has_armod() << "\n";
  h.armod( 0 );
  INFO_PRINT << "ar " << h.armod() << " " << h.has_armod() << "\n";
  INFO_PRINT << "size " << h.estimateSizeDynProps() << "\n";
  INFO_PRINT << "name " << h.itemname() << " " << h.has_itemname() << "\n";
  h.itemname( "hello world" );
  INFO_PRINT << "name " << h.itemname() << " " << h.has_itemname() << "\n";
  h.itemname( "" );
  INFO_PRINT << "name " << h.itemname() << " " << h.has_itemname() << "\n";
  INFO_PRINT << "size " << h.estimateSizeDynProps() << "\n";
}

void packet_test()
{
  using namespace Network;
  using namespace Network::PktHelper;
  auto debug = []( const PacketOut<PktOut_2F>& p ) {
    fmt::Writer w;
    for ( const u8& c : p->buffer )
    {
      w << fmt::hex( c ) << " ";
    }
    INFO_PRINT << w.str() << "\n";
  };
  auto test = []( const PacketOut<PktOut_2F>& p, const std::array<s8, 10>& a ) {

    if ( std::equal( std::begin( p->buffer ), std::end( p->buffer ), std::begin( a ) ) )
      INFO_PRINT << "success\n";
    else
    {
      INFO_PRINT << "failed\n";

      fmt::Writer w;
      for ( const u8& c : a )
      {
        w << fmt::hex( c ) << " ";
      }
      INFO_PRINT << w.str() << "\n";
    }
  };
  {
    PacketOut<PktOut_2F> p;  // size 10
    p->Write<s8>( 0x12 );
    p->Write<u8>( 0x21u );
    p->WriteFlipped<s8>( 0x12 );
    p->WriteFlipped<u8>( 0x21u );
    p->Write<u8>( 0u );
    debug( p );
    std::array<s8, 10> a{{0x2f, 0x12, 0x21, 0x12, 0x21, 0, 0, 0, 0, 0}};
    test( p, a );
  }
  {
    PacketOut<PktOut_2F> p;  // size 10
    p->Write<s16>( 0x1234 );
    p->Write<u16>( 0x4321u );
    p->WriteFlipped<s16>( 0x1234 );
    p->WriteFlipped<u16>( 0x4321u );
    p->Write<u8>( 0u );
    debug( p );
    std::array<s8, 10> a{{0x2f, 0x34, 0x12, 0x21, 0x43, 0x12, 0x34, 0x43, 0x21, 0}};
    test( p, a );
  }
  {
    PacketOut<PktOut_2F> p;  // size 10
    p->Write<s32>( 0x12344321 );
    p->Write<u32>( 0x12344321u );
    p->Write<u8>( 0u );
    debug( p );
    std::array<s8, 10> a{{0x2f, 0x21, 0x43, 0x34, 0x12, 0x21, 0x43, 0x34, 0x12, 0}};
    test( p, a );
  }
  {
    PacketOut<PktOut_2F> p;  // size 10
    p->WriteFlipped<s32>( 0x12344321 );
    p->WriteFlipped<u32>( 0x12344321u );
    p->Write<u8>( 0u );
    debug( p );
    std::array<s8, 10> a{{0x2f, 0x12, 0x34, 0x43, 0x21, 0x12, 0x34, 0x43, 0x21, 0}};
    test( p, a );
  }
  {
    PacketOut<PktOut_2F> p;  // size 10
    std::string s( "1234" );
    p->Write( s.c_str(), 4, false );
    u8 b[] = {0x12, 0x34, 0x43, 0x21};
    p->Write( b, 4 );
    p->Write<u8>( 0u );
    debug( p );
    std::array<s8, 10> a{{0x2f, 0x31, 0x32, 0x33, 0x34, 0x12, 0x34, 0x43, 0x21, 0}};
    test( p, a );
  }
}

}  // namespace Testing
}  // namespace Pol
