/** @file
 *
 * @par History
 */


#ifndef POL_PACKETS_H
#define POL_PACKETS_H

#include <cstring>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string.h>
#include <type_traits>

#include "../../clib/clib_endian.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/rawtypes.h"
#include "../../clib/spinlock.h"
#include "../../clib/strutil.h"
#include "../../plib/uconst.h"
#include "../layers.h"
#include "packetinterface.h"
#include "pktboth.h"
#include "pktbothid.h"
#include "pktdef.h"
#include "pktoutid.h"
#include "realms/realms.h"

namespace Pol
{
namespace Network
{
#define MAX_PACKETS_INSTANCES 100
namespace PacketWriterDefs
{
// "normal" packet queue
class PacketQueueSingle final : public PacketQueue
{
public:
  PacketQueueSingle();
  ~PacketQueueSingle() override;

private:
  PacketInterfaceQueue _packets;
  mutable Clib::SpinLock _lock;

public:
  PacketInterface* GetNext( u8 id, u16 sub = 0 ) override;
  void Add( PacketInterface* pkt ) override;
  size_t Count() const override { return _packets.size(); };
  size_t estimateSize() const override;
};

// packet with subs queue
class PacketQueueSubs final : public PacketQueue
{
public:
  PacketQueueSubs();
  ~PacketQueueSubs() override;

private:
  PacketInterfaceQueueMap _packets;
  mutable Clib::SpinLock _lock;

public:
  PacketInterface* GetNext( u8 id, u16 sub = 0 ) override;
  void Add( PacketInterface* pkt ) override;
  size_t Count() const override;
  bool HasSubs() const override { return true; };
  PacketInterfaceQueueMap* GetSubs() override { return &_packets; };
  size_t estimateSize() const override;
};

// wierdo generic template definitions for packets
namespace PktWriterTemplateSpecs
{
template <typename T>
struct WriteHelper
{
public:
  static void Write( T x, char buffer[], u16& offset );  // linker error
  static void WriteFlipped( T x, char buffer[], u16& offset );
};

template <>
struct WriteHelper<u32>
{
public:
  static void Write( u32 x, char buffer[], u16& offset )
  {
    std::memcpy( &buffer[offset], &x, sizeof( x ) );
    offset += 4;
  };
  static void WriteFlipped( u32 x, char buffer[], u16& offset )
  {
    x = cfBEu32( x );
    std::memcpy( &buffer[offset], &x, sizeof( x ) );
    offset += 4;
  };
};
template <>
struct WriteHelper<s32>
{
  static void Write( s32 x, char buffer[], u16& offset )
  {
    std::memcpy( &buffer[offset], &x, sizeof( x ) );
    offset += 4;
  };
  static void WriteFlipped( s32 x, char buffer[], u16& offset )
  {
    x = cfBEu32( x );
    std::memcpy( &buffer[offset], &x, sizeof( x ) );
    offset += 4;
  };
};
template <>
struct WriteHelper<u16>
{
  static void Write( u16 x, char buffer[], u16& offset )
  {
    std::memcpy( &buffer[offset], &x, sizeof( x ) );
    offset += 2;
  };
  static void WriteFlipped( u16 x, char buffer[], u16& offset )
  {
    x = cfBEu16( x );
    std::memcpy( &buffer[offset], &x, sizeof( x ) );
    offset += 2;
  };
};
template <>
struct WriteHelper<s16>
{
  static void Write( s16 x, char buffer[], u16& offset )
  {
    std::memcpy( &buffer[offset], &x, sizeof( x ) );
    offset += 2;
  };
  static void WriteFlipped( s16 x, char buffer[], u16& offset )
  {
    x = cfBEu16( x );
    std::memcpy( &buffer[offset], &x, sizeof( x ) );
    offset += 2;
  };
};
template <>
struct WriteHelper<u8>
{
  static void Write( u8 x, char buffer[], u16& offset ) { buffer[offset++] = x; }
  static void WriteFlipped( u8 x, char buffer[], u16& offset ) { buffer[offset++] = x; }
};
template <>
struct WriteHelper<s8>
{
  static void Write( s8 x, char buffer[], u16& offset ) { buffer[offset++] = x; };
  static void WriteFlipped( s8 x, char buffer[], u16& offset ) { buffer[offset++] = x; };
};
}  // namespace PktWriterTemplateSpecs

// "writer"class
template <u8 _id, u16 _size, u16 _sub = 0>
class PacketWriter : public PacketInterface
{
public:
  static const u8 ID = _id;
  static const u16 SUB = _sub;
  static const u16 SIZE = _size;
  char buffer[SIZE];
  char* getBuffer() override { return &buffer[offset]; };
  inline u8 getID() const override { return ID; };
  inline u16 getSize() const override { return SIZE; };
  size_t estimateSize() const override { return SIZE + sizeof( PacketInterface ); };
  // ---- Buffer Write Methods ----
  // N is the argument which will be static_cast to T
  // Two versions exists: T equals N only check if its integer or enum
  // T != N same check as above plus signed compare and during runtime max check
  // Todo instead of disallowing signed differences perform more magic to safely check the limits?
  template <class T, typename N>
  void Write( N x )
    requires std::is_same<T, N>::value
  {
    static_assert( std::is_integral<N>::value || std::is_enum<N>::value,
                   "Invalid argument type integral type is needed!" );
    passert_always_r( offset + sizeof( T ) <= SIZE, "pkt " + Clib::hexint( ID ) );
    PktWriterTemplateSpecs::WriteHelper<T>::Write( x, buffer, offset );
  };
  template <class T, typename N>
  void Write( N x )
    requires( !std::is_same<T, N>::value )
  {
    static_assert( std::is_integral<N>::value || std::is_enum<N>::value,
                   "Invalid argument type integral type is needed!" );
    static_assert( std::is_signed<T>::value == std::is_signed<N>::value,
                   "Signed/Unsigned missmatch!" );
    // passert_always_r((std::numeric_limits<T>::max() >= x), "Number is bigger then desired type!"
    // );
    passert_always_r( offset + sizeof( T ) <= SIZE, "pkt " + Clib::hexint( ID ) );
    if ( std::numeric_limits<T>::max() < x )  // dont let the shard crash, a warning is better
    {
      POLLOG_ERRORLN(
          "ERROR: Write: trying to write {} on offset {} for pkt {:#x}/{:#x} and only {} is "
          "allowed!",
          (int)x, offset, (int)ID, (int)SUB, std::numeric_limits<T>::max() );
      PktWriterTemplateSpecs::WriteHelper<T>::Write( std::numeric_limits<T>::max(), buffer,
                                                     offset );
    }
    else
      PktWriterTemplateSpecs::WriteHelper<T>::Write( static_cast<T>( x ), buffer, offset );
  };

  template <class T, typename N>
  void WriteFlipped( N x )
    requires std::is_same<T, N>::value
  {
    static_assert( std::is_integral<N>::value || std::is_enum<N>::value,
                   "Invalid argument type integral type is needed!" );
    passert_always_r( offset + sizeof( T ) <= SIZE, "pkt " + Clib::hexint( ID ) );
    PktWriterTemplateSpecs::WriteHelper<T>::WriteFlipped( x, buffer, offset );
  };

  template <class T, typename N>
  void WriteFlipped( N x )
    requires( !std::is_same<T, N>::value )
  {
    static_assert( std::is_integral<N>::value || std::is_enum<N>::value,
                   "Invalid argument type integral type is needed!" );
    static_assert( std::is_signed<T>::value == std::is_signed<N>::value,
                   "Signed/Unsigned missmatch!" );
    // passert_always_r((std::numeric_limits<T>::max() >= x), "Number is bigger then desired type!"
    // );
    passert_always_r( offset + sizeof( T ) <= SIZE, "pkt " + Clib::hexint( _id ) );
    if ( std::numeric_limits<T>::max() < x )  // dont let the shard crash, a warning is better
    {
      POLLOG_ERRORLN(
          "ERROR: WriteFlipped: trying to write {} on offset {} for pkt {:#x}/{:#x} and only {} "
          "is allowed!",
          (int)x, offset, (int)ID, (int)SUB, std::numeric_limits<T>::max() );
      PktWriterTemplateSpecs::WriteHelper<T>::WriteFlipped( std::numeric_limits<T>::max(), buffer,
                                                            offset );
    }
    else
      PktWriterTemplateSpecs::WriteHelper<T>::WriteFlipped( static_cast<T>( x ), buffer, offset );
  };

  void Write( const char* x, u16 len, bool nullterm = true )
  {
    if ( len < 1 )
      return;
    passert_always_r( offset + len <= SIZE, "pkt " + Clib::hexint( ID ) );
    strncpy( &buffer[offset], x, nullterm ? len - 1 : len );
    offset += len;
  };
  void Write( u8 x[], u16 len )
  {
    if ( len < 1 )
      return;
    passert_always_r( offset + len <= SIZE, "pkt " + Clib::hexint( ID ) );
    memcpy( &buffer[offset], x, len );
    offset += len;
  };
  void Write( const std::vector<u16>& x, bool nullterm = true )
  {
    passert_always_r( offset + x.size() * 2 <= SIZE, "pkt " + Clib::hexint( ID ) );
    if ( !x.empty() )
    {
      std::memcpy( &buffer[offset], x.data(), 2 * x.size() );
      offset += static_cast<u16>( x.size() * 2 );
    }
    if ( nullterm )
    {
      passert_always_r( offset + 2 <= SIZE, "pkt " + Clib::hexint( ID ) );
      offset += 2;
    }
  };
  void WriteFlipped( const std::vector<u16>& x, bool nullterm = true )
  {
    passert_always_r( offset + x.size() * 2 <= SIZE, "pkt " + Clib::hexint( ID ) );
    for ( const auto& c : x )
    {
      u16 tmp = ctBEu16( c );
      std::memcpy( &buffer[offset], &tmp, 2 );
      offset += 2;
    }
    if ( nullterm )
    {
      passert_always_r( offset + 2 <= SIZE, "pkt " + Clib::hexint( ID ) );
      offset += 2;
    }
  };
};

// "normal" pkt
template <u8 _id, u16 _size>
class PacketTemplate final : public PacketWriter<_id, _size>
{
public:
  PacketTemplate() { ReSetBuffer(); };
  void ReSetBuffer() override
  {
    memset( PacketWriter<_id, _size>::buffer, 0, _size );
    PacketWriter<_id, _size>::buffer[0] = _id;
    PacketWriter<_id, _size>::offset = 1;
  };
};

// sub packet
template <u8 _id, u16 _suboff, u16 _sub, u16 _size>
class PacketTemplateSub final : public PacketWriter<_id, _size, _sub>
{
public:
  PacketTemplateSub() { ReSetBuffer(); };
  void ReSetBuffer() override
  {
    memset( PacketWriter<_id, _size, _sub>::buffer, 0, _size );
    PacketWriter<_id, _size, _sub>::buffer[0] = _id;
    u16 sub = cfBEu16( _sub );
    std::memcpy( &PacketWriter<_id, _size, _sub>::buffer[_suboff], &sub, sizeof( sub ) );
    PacketWriter<_id, _size, _sub>::offset = 1;
  };
  inline u16 getSubID() const override { return _sub; };
};

// special def for encrypted buffer
template <u8 _id, u16 _size>
class EmptyBufferTemplate final : public PacketInterface
{
public:
  static const u8 ID = _id;
  static const u8 SUB = 0;
  static const u16 SIZE = _size;
  EmptyBufferTemplate() { ReSetBuffer(); };
  char buffer[SIZE];
  void ReSetBuffer() override
  {
    memset( buffer, 0, SIZE );
    offset = 0;
  };
  char* getBuffer() override { return &buffer[offset]; };
  inline u8 getID() const override { return ID; };
  inline u16 getSize() const override { return SIZE; };
  size_t estimateSize() const override { return SIZE + sizeof( PacketInterface ); };
};
}  // namespace PacketWriterDefs


// buffer for encrypted Data send with a dummy pktid
// NOTE: redefine id if pkt 0x0 ever gets send
#define ENCRYPTEDPKTBUFFER static_cast<u8>( 0 )

// Packet defs start
using EncryptedPktBuffer = PacketWriterDefs::EmptyBufferTemplate<static_cast<u8>( 0 ), 65535>;

using PktOut_0B = PacketWriterDefs::PacketTemplate<Core::PKTOUT_0B_ID, 7>;
using PktOut_11 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_11_ID, 121>;
using PktOut_17 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_17_ID, 12>;
using PktOut_1A = PacketWriterDefs::PacketTemplate<Core::PKTOUT_1A_ID, 20>;
using PktOut_1B = PacketWriterDefs::PacketTemplate<Core::PKTOUT_1B_ID, 37>;
using PktOut_1C = PacketWriterDefs::PacketTemplate<Core::PKTOUT_1C_ID, 44 + 200 + 1>;
using PktOut_1D = PacketWriterDefs::PacketTemplate<Core::PKTOUT_1D_ID, 5>;
using PktOut_20 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_20_ID, 19>;
using PktOut_21 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_21_ID, 8>;
using PktOut_22 = PacketWriterDefs::PacketTemplate<Core::PKTBI_22_APPROVED_ID, 3>;
// HSA 9 old 7
using PktOut_24 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_24_ID, 9>;
using PktOut_25 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_25_ID, 21>;
using PktOut_27 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_27_ID, 2>;
using PktOut_29 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_29_ID, 1>;
using PktOut_2C = PacketWriterDefs::PacketTemplate<Core::PKTBI_2C_ID, 2>;
using PktOut_2D = PacketWriterDefs::PacketTemplate<Core::PKTOUT_2D_ID, 17>;
using PktOut_2E = PacketWriterDefs::PacketTemplate<Core::PKTOUT_2E_ID, 15>;
using PktOut_2F = PacketWriterDefs::PacketTemplate<Core::PKTOUT_2F_ID, 10>;
using PktOut_3A = PacketWriterDefs::PacketTemplate<Core::PKTBI_3A_ID, 6 + 100 * 9>;
using PktOut_3B = PacketWriterDefs::PacketTemplate<Core::PKTBI_3B_ID, 8>;
using PktOut_3C = PacketWriterDefs::PacketTemplate<Core::PKTOUT_3C_ID, 5 + 3200 * 20>;
using PktOut_4F = PacketWriterDefs::PacketTemplate<Core::PKTOUT_4F_ID, 2>;
using PktOut_53 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_53_ID, 2>;
using PktOut_54 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_54_ID, 12>;
using PktOut_55 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_55_ID, 1>;
using PktOut_56 = PacketWriterDefs::PacketTemplate<Core::PKTBI_56_ID, 11>;
using PktOut_65 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_65_ID, 4>;
using PktOut_66 = PacketWriterDefs::PacketTemplate<Core::PKTBI_66_ID, 65535>;
using PktOut_6C = PacketWriterDefs::PacketTemplate<Core::PKTBI_6C_ID, 19>;
using PktOut_6D = PacketWriterDefs::PacketTemplate<Core::PKTOUT_6D_ID, 3>;
using PktOut_6E = PacketWriterDefs::PacketTemplate<Core::PKTOUT_6E_ID, 14>;
using PktOut_6F = PacketWriterDefs::PacketTemplate<Core::PKTBI_6F_ID, 47>;
using PktOut_70 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_70_ID, 28>;
using PktOut_72 = PacketWriterDefs::PacketTemplate<Core::PKTBI_72_ID, 5>;
using PktOut_74 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_74_ID, 65535>;
using PktOut_76 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_76_ID, 16>;
using PktOut_77 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_77_ID, 17>;
using PktOut_78 =
    PacketWriterDefs::PacketTemplate<Core::PKTOUT_78_ID, 19 + ( 9 * Core::HIGHEST_LAYER ) + 4>;
using PktOut_7C = PacketWriterDefs::PacketTemplate<Core::PKTOUT_7C_ID, 2000>;
using PktOut_82 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_82_ID, 2>;
using PktOut_88 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_88_ID, 66>;
using PktOut_89 =
    PacketWriterDefs::PacketTemplate<Core::PKTOUT_89_ID, 7 + ( 5 * ( Core::NUM_LAYERS + 1 ) ) + 1>;
using PktOut_8C = PacketWriterDefs::PacketTemplate<Core::PKTOUT_8C_ID, 11>;
using PktOut_90 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_90_ID, 19>;
using PktOut_93 = PacketWriterDefs::PacketTemplate<Core::PKTBI_93_ID, 99>;
using PktOut_95 = PacketWriterDefs::PacketTemplate<Core::PKTBI_95_ID, 9>;
using PktOut_98 = PacketWriterDefs::PacketTemplate<Core::PKTBI_98_OUT_ID, 37>;
// HSA 30 old 26
using PktOut_99 = PacketWriterDefs::PacketTemplate<Core::PKTBI_99_ID, 30>;
using PktOut_9A = PacketWriterDefs::PacketTemplate<Core::PKTBI_9A_ID, 16>;
using PktOut_9E = PacketWriterDefs::PacketTemplate<Core::PKTOUT_9E_ID, 65535>;
using PktOut_A1 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_A1_ID, 9>;
using PktOut_A2 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_A2_ID, 9>;
using PktOut_A3 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_A3_ID, 9>;
using PktOut_A5 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_A5_ID, 200 + 4>;
using PktOut_A6 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_A6_ID, 10010>;
using PktOut_A8 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_A8_ID, 2000>;
using PktOut_A9 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_A9_ID, 10000>;
using PktOut_AA = PacketWriterDefs::PacketTemplate<Core::PKTOUT_AA_ID, 5>;
using PktOut_AB = PacketWriterDefs::PacketTemplate<Core::PKTOUT_AB_ID, 531>;
using PktOut_AE = PacketWriterDefs::PacketTemplate<Core::PKTOUT_AE_ID, ( 200 + 1 ) * 2 + 48>;
using PktOut_AF = PacketWriterDefs::PacketTemplate<Core::PKTOUT_AF_ID, 13>;
using PktOut_B0 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_B0_ID, 65535>;
using PktOut_B7 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_B7_ID, 521>;
using PktOut_B8 = PacketWriterDefs::PacketTemplate<Core::PKTBI_B8_OUT_ID, 7 + 5 * ( 200 + 1 )>;
using PktOut_B9 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_B9_ID, 5>;
// HSA 10 old 6
using PktOut_BA = PacketWriterDefs::PacketTemplate<Core::PKTOUT_BA_ID, 10>;
using PktOut_BC = PacketWriterDefs::PacketTemplate<Core::PKTOUT_BC_ID, 3>;

using PktOut_BF_Sub4 = PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3,
                                                           Core::PKTBI_BF::TYPE_CLOSE_GUMP, 5 + 8>;
using PktOut_BF_Sub6 =
    PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3, Core::PKTBI_BF::TYPE_PARTY_SYSTEM,
                                        65535>;
using PktOut_BF_Sub8 = PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3,
                                                           Core::PKTBI_BF::TYPE_CURSOR_HUE, 5 + 1>;
using PktOut_BF_Sub10 =
    PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3, Core::PKTBI_BF::TYPE_OBJECT_CACHE,
                                        5 + 8>;
using PktOut_BF_Sub14 =
    PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3,
                                        Core::PKTBI_BF::TYPE_DISPLAY_POPUP_MENU, 65535>;
using PktOut_BF_Sub16 =
    PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3, Core::PKTBI_BF::TYPE_CLOSE_WINDOW,
                                        5 + 8>;
using PktOut_BF_Sub18 =
    PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3, Core::PKTBI_BF::TYPE_ENABLE_MAP_DIFFS,
                                        5 + 4 + 6 * 8>;
using PktOut_BF_Sub19 =
    PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3,
                                        Core::PKTBI_BF::TYPE_EXTENDED_STATS_OUT, 5 + 7>;
using PktOut_BF_Sub1B =
    PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3, Core::PKTBI_BF::TYPE_NEW_SPELLBOOK,
                                        5 + 18>;
using PktOut_BF_Sub1D =
    PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3,
                                        Core::PKTBI_BF::TYPE_CUSTOM_HOUSE_SHORT, 5 + 8>;
using PktOut_BF_Sub20 =
    PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3,
                                        Core::PKTBI_BF::TYPE_ACTIVATE_CUSTOM_HOUSE_TOOL, 5 + 12>;
using PktOut_BF_Sub22 =
    PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3, Core::PKTBI_BF::TYPE_DAMAGE, 5 + 6>;
using PktOut_BF_Sub2A =
    PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3,
                                        Core::PKTBI_BF::TYPE_CHARACTER_RACE_CHANGER, 5 + 2>;

using PktOut_C1 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_C1_ID, 48 + ( 200 + 1 ) + 2>;
using PktOut_C2 = PacketWriterDefs::PacketTemplate<Core::PKTBI_C2_ID, 21>;
using PktOut_C7 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_C7_ID, 49>;
using PktOut_C8 = PacketWriterDefs::PacketTemplate<Core::PKTBI_C8_ID, 2>;
using PktOut_CC =
    PacketWriterDefs::PacketTemplate<Core::PKTOUT_CC_ID, 49 + ( 200 + 1 ) * 2 + 200 + 1>;
using PktOut_D6 = PacketWriterDefs::PacketTemplate<Core::PKTBI_D6_OUT_ID, 65535>;
using PktOut_DC = PacketWriterDefs::PacketTemplate<Core::PKTOUT_DC_ID, 9>;
using PktOut_DD = PacketWriterDefs::PacketTemplate<Core::PKTOUT_DD_ID, 65535>;
using PktOut_DF = PacketWriterDefs::PacketTemplate<Core::PKTOUT_DF_ID, 65535>;
using PktOut_E2 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_E2_ID, 10>;
using PktOut_E3 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_E3_ID, 77>;
using PktOut_F0_Sub01 =
    PacketWriterDefs::PacketTemplateSub<Core::PKTBI_F0_ID, 1, Core::PKTBI_F0::QUERY_PARTY, 65535>;
using PktOut_F0_Sub02 =
    PacketWriterDefs::PacketTemplateSub<Core::PKTBI_F0_ID, 1, Core::PKTBI_F0::QUERY_GUILD, 65535>;
// HSA 26 old 24
using PktOut_F3 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_F3_ID, 26>;
using PktOut_F5 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_F5_ID, 21>;
using PktOut_F6 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_F6_ID, 65535>;
using PktOut_F7 = PacketWriterDefs::PacketTemplate<Core::PKTOUT_F7_ID, 65535>;
// Packet defs end
}  // namespace Network
}  // namespace Pol
#endif
