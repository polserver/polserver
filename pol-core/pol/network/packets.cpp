/** @file
 *
 * @par History
 *
 * @note New packet add:
 * - PacketsSingleton::PacketsSingleton() add queue for it
 * - PacketInterface* GetPacket(u8 id, u16 sub) add pkt create
 */


#include "packets.h"

#include "../../clib/rawtypes.h"
#include "../../clib/spinlock.h"
#include "../../clib/stlutil.h"
#include "packethelper.h"
#include "packetinterface.h"
#include "pktboth.h"
#include "pktbothid.h"

namespace Pol
{
namespace Network
{
using namespace PktHelper;
using namespace PacketWriterDefs;

/** @class PacketsSingleton
 * Central class that holds every pkt object
 *
 * This way client doesnt need to create pkts, just requests a object and
 * after sending readds it
 */

PacketsSingleton::PacketsSingleton()
{
  using namespace Core;
  // insert packet queues at first creation
  packets.insert( PacketQueuePair( ENCRYPTEDPKTBUFFER, new PacketQueueSingle() ) );

  packets.insert( PacketQueuePair( PKTOUT_0B_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_11_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_17_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_1A_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_1B_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_1C_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_1D_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_20_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_21_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_22_APPROVED_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_24_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_25_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_27_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_29_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_2C_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_2D_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_2E_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_2F_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_3A_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_3B_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_3C_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_4F_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_53_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_54_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_55_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_56_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_65_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_66_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_6C_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_6D_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_6E_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_6F_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_70_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_72_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_74_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_76_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_77_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_78_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_7C_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_82_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_88_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_89_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_8C_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_90_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_93_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_95_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_98_OUT_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_99_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_9A_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_9E_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_A1_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_A2_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_A3_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_A5_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_A6_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_A8_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_A9_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_AA_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_AB_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_AE_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_AF_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_B0_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_B7_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_B8_OUT_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_B9_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_BA_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_BC_ID, new PacketQueueSingle() ) );

  packets.insert( PacketQueuePair( PKTBI_BF_ID, new PacketQueueSubs() ) );

  packets.insert( PacketQueuePair( PKTOUT_C1_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_C2_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_C7_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_C8_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_CC_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_D6_OUT_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_DC_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_DD_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_DF_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_E2_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_E3_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTBI_F0_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_F3_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_F5_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_F6_ID, new PacketQueueSingle() ) );
  packets.insert( PacketQueuePair( PKTOUT_F7_ID, new PacketQueueSingle() ) );
}

PacketsSingleton::~PacketsSingleton()
{
  for ( auto& pkts : packets )
  {
    delete pkts.second;
  }
  packets.clear();
}

PacketInterface* PacketsSingleton::getPacket( u8 id, u16 sub )
{
  PacketQueueMap::iterator itr = packets.find( id );
  if ( itr != packets.end() )
    return itr->second->GetNext( id, sub );
  throw std::runtime_error( "Request of undefined Packet: " + Clib::hexint( id ) + "-" +
                            Clib::hexint( sub ) );
}

void PacketsSingleton::ReAddPacket( PacketInterface* pkt )
{
  PacketQueueMap::iterator itr = packets.find( pkt->getID() );
  if ( itr != packets.end() )
    itr->second->Add( pkt );
  else
    delete pkt;
}

size_t PacketsSingleton::estimateSize() const
{
  size_t size = sizeof( PacketsSingleton ) + Clib::memsize( packets );
  for ( const auto& pkts : packets )
  {
    size += pkts.second->estimateSize();
  }
  return size;
}

PacketQueueSingle::PacketQueueSingle() : _packets(), _lock() {}
PacketInterface* PacketQueueSingle::GetNext( u8 id, u16 /*sub*/ )
{
  // critical start
  Clib::SpinLockGuard lock( _lock );
  PacketInterface* pkt;
  if ( !_packets.empty() )
  {
    pkt = _packets.front();  // get next one
    _packets.pop();          // and remove it from queue
    pkt->ReSetBuffer();
  }
  else
    pkt = GetPacket( id );
  return pkt;
  // critical end
}

PacketQueueSingle::~PacketQueueSingle()
{
  Clib::SpinLockGuard lock( _lock );
  while ( !_packets.empty() )
  {
    PacketInterface* pkt = _packets.front();
    _packets.pop();
    delete pkt;
  }
}

void PacketQueueSingle::Add( PacketInterface* pkt )
{
  Clib::SpinLockGuard lock( _lock );
  if ( _packets.size() > MAX_PACKETS_INSTANCES )  // enough?
    delete pkt;
  else
  {
    _packets.push( pkt );  // readd it
  }
}

size_t PacketQueueSingle::estimateSize() const
{
  Clib::SpinLockGuard lock( _lock );
  size_t size = sizeof( PacketQueueSingle );
  if ( !_packets.empty() )
    size += _packets.front()->estimateSize() * _packets.size();
  return size;
}

PacketQueueSubs::PacketQueueSubs() : _packets(), _lock() {}
PacketQueueSubs::~PacketQueueSubs()
{
  Clib::SpinLockGuard lock( _lock );
  for ( auto& pkts : _packets )
  {
    while ( !pkts.second.empty() )
    {
      PacketInterface* pkt = pkts.second.front();
      pkts.second.pop();
      delete pkt;
    }
  }
  _packets.clear();
}

PacketInterface* PacketQueueSubs::GetNext( u8 id, u16 sub )
{
  // critical start
  Clib::SpinLockGuard lock( _lock );
  PacketInterface* pkt;
  if ( !_packets.empty() )
  {
    PacketInterfaceQueueMap::iterator itr = _packets.find( sub );
    if ( itr != _packets.end() )
    {
      if ( !itr->second.empty() )
      {
        pkt = itr->second.front();  // get next one
        itr->second.pop();          // and remove it from queue
        pkt->ReSetBuffer();
        return pkt;
      }
    }
  }
  pkt = GetPacket( id, sub );
  return pkt;
  // critical end
}

void PacketQueueSubs::Add( PacketInterface* pkt )
{
  u16 sub = pkt->getSubID();
  // critical start
  Clib::SpinLockGuard lock( _lock );
  PacketInterfaceQueueMap::iterator itr = _packets.find( sub );
  if ( itr != _packets.end() )
  {
    if ( itr->second.size() > MAX_PACKETS_INSTANCES )  // enough?
      delete pkt;
    else
      itr->second.push( pkt );  // readd it
  }
  else  // new sub
  {
    PacketInterfaceQueue qu;
    qu.push( pkt );
    _packets.insert( PacketInterfaceQueuePair( sub, qu ) );
  }
  // critical end
}

size_t PacketQueueSubs::Count() const
{
  Clib::SpinLockGuard lock( _lock );
  size_t count = 0;
  for ( const auto& pkts : _packets )
  {
    count += pkts.second.size();
  }
  return count;
}

size_t PacketQueueSubs::estimateSize() const
{
  Clib::SpinLockGuard lock( _lock );
  size_t size = sizeof( PacketQueueSubs ) + Clib::memsize( _packets );
  for ( const auto& pkts : _packets )
  {
    if ( !pkts.second.empty() )
      size += pkts.second.front()->estimateSize() * pkts.second.size();
  }
  return size;
}

namespace PktHelper
{
// creates new packets
PacketInterface* GetPacket( u8 id, u16 sub )
{
  using namespace Core;
  switch ( id )
  {
  case ENCRYPTEDPKTBUFFER:
    return new EncryptedPktBuffer();
  case PKTOUT_0B_ID:
    return new PktOut_0B();
  case PKTOUT_11_ID:
    return new PktOut_11();
  case PKTOUT_17_ID:
    return new PktOut_17();
  case PKTOUT_1A_ID:
    return new PktOut_1A();
  case PKTOUT_1B_ID:
    return new PktOut_1B();
  case PKTOUT_1C_ID:
    return new PktOut_1C();
  case PKTOUT_1D_ID:
    return new PktOut_1D();
  case PKTOUT_20_ID:
    return new PktOut_20();
  case PKTOUT_21_ID:
    return new PktOut_21();
  case PKTBI_22_APPROVED_ID:
    return new PktOut_22();
  case PKTOUT_24_ID:
    return new PktOut_24();
  case PKTOUT_25_ID:
    return new PktOut_25();
  case PKTOUT_27_ID:
    return new PktOut_27();
  case PKTOUT_29_ID:
    return new PktOut_29();
  case PKTBI_2C_ID:
    return new PktOut_2C();
  case PKTOUT_2D_ID:
    return new PktOut_2D();
  case PKTOUT_2E_ID:
    return new PktOut_2E();
  case PKTOUT_2F_ID:
    return new PktOut_2F();
  case PKTBI_3A_ID:
    return new PktOut_3A();
  case PKTBI_3B_ID:
    return new PktOut_3B();
  case PKTOUT_3C_ID:
    return new PktOut_3C();
  case PKTOUT_4F_ID:
    return new PktOut_4F();
  case PKTOUT_53_ID:
    return new PktOut_53();
  case PKTOUT_54_ID:
    return new PktOut_54();
  case PKTOUT_55_ID:
    return new PktOut_55();
  case PKTBI_56_ID:
    return new PktOut_56();
  case PKTOUT_65_ID:
    return new PktOut_65();
  case PKTBI_66_ID:
    return new PktOut_66();
  case PKTBI_6C_ID:
    return new PktOut_6C();
  case PKTOUT_6D_ID:
    return new PktOut_6D();
  case PKTOUT_6E_ID:
    return new PktOut_6E();
  case PKTBI_6F_ID:
    return new PktOut_6F();
  case PKTOUT_70_ID:
    return new PktOut_70();
  case PKTBI_72_ID:
    return new PktOut_72();
  case PKTOUT_74_ID:
    return new PktOut_74();
  case PKTOUT_76_ID:
    return new PktOut_76();
  case PKTOUT_77_ID:
    return new PktOut_77();
  case PKTOUT_78_ID:
    return new PktOut_78();
  case PKTOUT_7C_ID:
    return new PktOut_7C();
  case PKTOUT_82_ID:
    return new PktOut_82();
  case PKTOUT_88_ID:
    return new PktOut_88();
  case PKTOUT_89_ID:
    return new PktOut_89();
  case PKTOUT_8C_ID:
    return new PktOut_8C();
  case PKTOUT_90_ID:
    return new PktOut_90();
  case PKTBI_93_ID:
    return new PktOut_93();
  case PKTBI_95_ID:
    return new PktOut_95();
  case PKTBI_98_OUT_ID:
    return new PktOut_98();
  case PKTBI_99_ID:
    return new PktOut_99();
  case PKTBI_9A_ID:
    return new PktOut_9A();
  case PKTOUT_9E_ID:
    return new PktOut_9E();
  case PKTOUT_A1_ID:
    return new PktOut_A1();
  case PKTOUT_A2_ID:
    return new PktOut_A2();
  case PKTOUT_A3_ID:
    return new PktOut_A3();
  case PKTOUT_A5_ID:
    return new PktOut_A5();
  case PKTOUT_A6_ID:
    return new PktOut_A6();
  case PKTOUT_A8_ID:
    return new PktOut_A8();
  case PKTOUT_A9_ID:
    return new PktOut_A9();
  case PKTOUT_AA_ID:
    return new PktOut_AA();
  case PKTOUT_AB_ID:
    return new PktOut_AB();
  case PKTOUT_AE_ID:
    return new PktOut_AE();
  case PKTOUT_AF_ID:
    return new PktOut_AF();
  case PKTOUT_B0_ID:
    return new PktOut_B0();
  case PKTOUT_B7_ID:
    return new PktOut_B7();
  case PKTBI_B8_OUT_ID:
    return new PktOut_B8();
  case PKTOUT_B9_ID:
    return new PktOut_B9();
  case PKTOUT_BA_ID:
    return new PktOut_BA();
  case PKTOUT_BC_ID:
    return new PktOut_BC();

  case PKTBI_BF_ID:
  {
    switch ( sub )
    {
    case Core::PKTBI_BF::TYPE_CLOSE_GUMP:
      return new PktOut_BF_Sub4();
    case Core::PKTBI_BF::TYPE_PARTY_SYSTEM:
      return new PktOut_BF_Sub6();
    case Core::PKTBI_BF::TYPE_CURSOR_HUE:
      return new PktOut_BF_Sub8();
    case Core::PKTBI_BF::TYPE_OBJECT_CACHE:
      return new PktOut_BF_Sub10();
    case Core::PKTBI_BF::TYPE_DISPLAY_POPUP_MENU:
      return new PktOut_BF_Sub14();
    case Core::PKTBI_BF::TYPE_CLOSE_WINDOW:
      return new PktOut_BF_Sub16();
    case Core::PKTBI_BF::TYPE_ENABLE_MAP_DIFFS:
      return new PktOut_BF_Sub18();
    case Core::PKTBI_BF::TYPE_EXTENDED_STATS_OUT:
      return new PktOut_BF_Sub19();
    case Core::PKTBI_BF::TYPE_NEW_SPELLBOOK:
      return new PktOut_BF_Sub1B();
    case Core::PKTBI_BF::TYPE_CUSTOM_HOUSE_SHORT:
      return new PktOut_BF_Sub1D();
    case Core::PKTBI_BF::TYPE_ACTIVATE_CUSTOM_HOUSE_TOOL:
      return new PktOut_BF_Sub20();
    case Core::PKTBI_BF::TYPE_DAMAGE:
      return new PktOut_BF_Sub22();
    case Core::PKTBI_BF::TYPE_CHARACTER_RACE_CHANGER:
      return new PktOut_BF_Sub2A();
    default:
      throw std::runtime_error( "Request of undefined Packet: " + Clib::hexint( id ) + "-" +
                                Clib::hexint( sub ) );
    }
  }
  case PKTOUT_C1_ID:
    return new PktOut_C1();
  case PKTBI_C2_ID:
    return new PktOut_C2();
  case PKTOUT_C7_ID:
    return new PktOut_C7();
  case PKTBI_C8_ID:
    return new PktOut_C8();
  case PKTOUT_CC_ID:
    return new PktOut_CC();
  case PKTBI_D6_OUT_ID:
    return new PktOut_D6();
  case PKTOUT_DC_ID:
    return new PktOut_DC();
  case PKTOUT_DD_ID:
    return new PktOut_DD();
  case PKTOUT_DF_ID:
    return new PktOut_DF();
  case PKTOUT_E2_ID:
    return new PktOut_E2();
  case PKTOUT_E3_ID:
    return new PktOut_E3();
  case PKTOUT_F3_ID:
    return new PktOut_F3();
  case PKTOUT_F5_ID:
    return new PktOut_F5();
  case PKTOUT_F6_ID:
    return new PktOut_F6();
  case PKTOUT_F7_ID:
    return new PktOut_F7();
  case PKTBI_F0_ID:
    switch ( sub )
    {
    case Core::PKTBI_F0::QUERY_PARTY:
      return new PktOut_F0_Sub01();
    case Core::PKTBI_F0::QUERY_GUILD:
      return new PktOut_F0_Sub02();
    default:
      throw std::runtime_error( "Request of undefined Packet: " + Clib::hexint( id ) + "-" +
                                Clib::hexint( sub ) );
    }
  default:
    throw std::runtime_error( "Request of undefined Packet: " + Clib::hexint( id ) + "-" +
                              Clib::hexint( sub ) );
  }
}
}  // namespace PktHelper
}  // namespace Network
}  // namespace Pol
