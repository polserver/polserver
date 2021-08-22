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
#include "testenv.h"

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
  auto debug = []( const PacketOut<PktOut_2F>& p )
  {
    fmt::Writer w;
    for ( const u8& c : p->buffer )
    {
      w << fmt::hex( c ) << " ";
    }
    INFO_PRINT << w.str() << "\n";
  };
  auto test = [&]( const PacketOut<PktOut_2F>& p, const std::array<s8, 10>& a )
  {
    if ( std::equal( std::begin( p->buffer ), std::end( p->buffer ), std::begin( a ) ) )
      UnitTest::inc_successes();
    else
    {
      UnitTest::inc_failures();
      INFO_PRINT << "failed\n";
      debug( p );
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
    std::array<s8, 10> a{ { 0x2f, 0x12, 0x21, 0x12, 0x21, 0, 0, 0, 0, 0 } };
    test( p, a );
  }
  {
    PacketOut<PktOut_2F> p;  // size 10
    p->Write<s16>( 0x1234 );
    p->Write<u16>( 0x4321u );
    p->WriteFlipped<s16>( 0x1234 );
    p->WriteFlipped<u16>( 0x4321u );
    p->Write<u8>( 0u );
    std::array<s8, 10> a{ { 0x2f, 0x34, 0x12, 0x21, 0x43, 0x12, 0x34, 0x43, 0x21, 0 } };
    test( p, a );
  }
  {
    PacketOut<PktOut_2F> p;  // size 10
    p->Write<s32>( 0x12344321 );
    p->Write<u32>( 0x12344321u );
    p->Write<u8>( 0u );
    std::array<s8, 10> a{ { 0x2f, 0x21, 0x43, 0x34, 0x12, 0x21, 0x43, 0x34, 0x12, 0 } };
    test( p, a );
  }
  {
    PacketOut<PktOut_2F> p;  // size 10
    p->WriteFlipped<s32>( 0x12344321 );
    p->WriteFlipped<u32>( 0x12344321u );
    p->Write<u8>( 0u );
    std::array<s8, 10> a{ { 0x2f, 0x12, 0x34, 0x43, 0x21, 0x12, 0x34, 0x43, 0x21, 0 } };
    test( p, a );
  }
  {
    PacketOut<PktOut_2F> p;  // size 10
    std::string s( "1234" );
    p->Write( s.c_str(), 4, false );
    u8 b[] = { 0x12, 0x34, 0x43, 0x21 };
    p->Write( b, 4 );
    p->Write<u8>( 0u );
    std::array<s8, 10> a{ { 0x2f, 0x31, 0x32, 0x33, 0x34, 0x12, 0x34, 0x43, 0x21, 0 } };
    test( p, a );
  }
}

void test_splitnamevalue( const std::string& istr, const std::string& exp_pn,
                          const std::string& exp_pv )
{
  std::string pn, pv;
  Clib::splitnamevalue( istr, pn, pv );
  if ( pn != exp_pn || pv != exp_pv )
  {
    INFO_PRINT << "splitnamevalue( \"" << istr << "\" ) fails!\n";
    UnitTest::inc_failures();
  }
  else
    UnitTest::inc_successes();
}

void test_splitnamevalue()
{
  test_splitnamevalue( "a b", "a", "b" );
  test_splitnamevalue( "av bx", "av", "bx" );
  test_splitnamevalue( "nm=valu", "nm", "valu" );
  test_splitnamevalue( "nm:valu", "nm:valu", "" );
  test_splitnamevalue( "nm", "nm", "" );
  test_splitnamevalue( "  nm", "nm", "" );
  test_splitnamevalue( "  nm  ", "nm", "" );
  test_splitnamevalue( "  nm valu", "nm", "valu" );
  test_splitnamevalue( "  nm   value   ", "nm", "value" );
  test_splitnamevalue( "  nm  value is multiple words", "nm", "value is multiple words" );
  test_splitnamevalue( "  nm  value is multiple words\t ", "nm", "value is multiple words" );
}

void test_dqs( const std::string& in, const std::string& out )
{
  std::string tmp = in;
  Clib::decodequotedstring( tmp );
  if ( tmp != out )
  {
    INFO_PRINT << "decodequotedstring( " << in << " ) fails!\n";
    UnitTest::inc_failures();
  }
  else
    UnitTest::inc_successes();

  Clib::encodequotedstring( tmp );
  if ( tmp != in )
  {
    INFO_PRINT << "encodequotedstring( " << out << " ) fails!\n";
    UnitTest::inc_failures();
  }
  else
    UnitTest::inc_successes();
}

void test_convertquotedstring()
{
  test_dqs( "\"hi\"", "hi" );
  test_dqs( "\"hi \"", "hi " );
  test_dqs( "\" hi \"", " hi " );
  test_dqs( "\" \\\"hi\"", " \"hi" );
}

}  // namespace Testing
}  // namespace Pol
