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
  virtual ~PacketQueueSingle();

private:
  PacketInterfaceQueue _packets;
  mutable Clib::SpinLock _lock;

public:
  virtual PacketInterface* GetNext( u8 id, u16 sub = 0 ) override;
  virtual void Add( PacketInterface* pkt ) override;
  virtual size_t Count() const override { return _packets.size(); };
  virtual size_t estimateSize() const override;
};

// packet with subs queue
class PacketQueueSubs final : public PacketQueue
{
public:
  PacketQueueSubs();
  virtual ~PacketQueueSubs();

private:
  PacketInterfaceQueueMap _packets;
  mutable Clib::SpinLock _lock;

public:
  virtual PacketInterface* GetNext( u8 id, u16 sub = 0 ) override;
  virtual void Add( PacketInterface* pkt ) override;
  virtual size_t Count() const override;
  virtual bool HasSubs() const override { return true; };
  virtual PacketInterfaceQueueMap* GetSubs() override { return &_packets; };
  virtual size_t estimateSize() const override;
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
  virtual char* getBuffer() override { return &buffer[offset]; };
  virtual inline u8 getID() const override { return ID; };
  virtual inline u16 getSize() const override { return SIZE; };
  virtual size_t estimateSize() const override { return SIZE + sizeof( PacketInterface ); };
  // ---- Buffer Write Methods ----
  // N is the argument which will be static_cast to T
  // Two versions exists: T equals N only check if its integer or enum
  // T != N same check as above plus signed compare and during runtime max check
  // Todo instead of disallowing signed differences perform more magic to safely check the limits?
  template <class T, typename N>
  typename std::enable_if<std::is_same<T, N>::value, void>::type Write( N x )
  {
    static_assert( std::is_integral<N>::value || std::is_enum<N>::value,
                   "Invalid argument type integral type is needed!" );
    passert_always_r( offset + sizeof( T ) <= SIZE, "pkt " + Clib::hexint( ID ) );
    PktWriterTemplateSpecs::WriteHelper<T>::Write( x, buffer, offset );
  };
  template <class T, typename N>
  typename std::enable_if<!std::is_same<T, N>::value, void>::type Write( N x )
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
  typename std::enable_if<std::is_same<T, N>::value, void>::type WriteFlipped( N x )
  {
    static_assert( std::is_integral<N>::value || std::is_enum<N>::value,
                   "Invalid argument type integral type is needed!" );
    passert_always_r( offset + sizeof( T ) <= SIZE, "pkt " + Clib::hexint( ID ) );
    PktWriterTemplateSpecs::WriteHelper<T>::WriteFlipped( x, buffer, offset );
  };

  template <class T, typename N>
  typename std::enable_if<!std::is_same<T, N>::value, void>::type WriteFlipped( N x )
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
    std::memcpy( &buffer[offset], x.data(), 2 * x.size() );
    offset += static_cast<u16>( x.size() * 2 );
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
  virtual void ReSetBuffer() override
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
  virtual void ReSetBuffer() override
  {
    memset( PacketWriter<_id, _size, _sub>::buffer, 0, _size );
    PacketWriter<_id, _size, _sub>::buffer[0] = _id;
    u16 sub = cfBEu16( _sub );
    std::memcpy( &PacketWriter<_id, _size, _sub>::buffer[_suboff], &sub, sizeof( sub ) );
    PacketWriter<_id, _size, _sub>::offset = 1;
  };
  virtual inline u16 getSubID() const override { return _sub; };
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
  virtual void ReSetBuffer() override
  {
    memset( buffer, 0, SIZE );
    offset = 0;
  };
  virtual char* getBuffer() override { return &buffer[offset]; };
  virtual inline u8 getID() const override { return ID; };
  virtual inline u16 getSize() const override { return SIZE; };
  virtual size_t estimateSize() const override { return SIZE + sizeof( PacketInterface ); };
};
}  // namespace PacketWriterDefs


// buffer for encrypted Data send with a dummy pktid
// NOTE: redefine id if pkt 0x0 ever gets send
#define ENCRYPTEDPKTBUFFER static_cast<u8>( 0 )

// Packet defs start
typedef PacketWriterDefs::EmptyBufferTemplate<ENCRYPTEDPKTBUFFER, 0xFFFF> EncryptedPktBuffer;

typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_0B_ID, 7> PktOut_0B;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_11_ID, 121> PktOut_11;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_17_ID, 12> PktOut_17;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_1A_ID, 20> PktOut_1A;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_1B_ID, 37> PktOut_1B;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_1C_ID, 44 + SPEECH_MAX_LEN + 1> PktOut_1C;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_1D_ID, 5> PktOut_1D;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_20_ID, 19> PktOut_20;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_21_ID, 8> PktOut_21;
typedef PacketWriterDefs::PacketTemplate<Core::PKTBI_22_APPROVED_ID, 3> PktOut_22;
// HSA 9 old 7
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_24_ID, 9> PktOut_24;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_25_ID, 21> PktOut_25;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_27_ID, 2> PktOut_27;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_29_ID, 1> PktOut_29;
typedef PacketWriterDefs::PacketTemplate<Core::PKTBI_2C_ID, 2> PktOut_2C;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_2D_ID, 17> PktOut_2D;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_2E_ID, 15> PktOut_2E;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_2F_ID, 10> PktOut_2F;
typedef PacketWriterDefs::PacketTemplate<Core::PKTBI_3A_ID, 6 + 100 * 9> PktOut_3A;
typedef PacketWriterDefs::PacketTemplate<Core::PKTBI_3B_ID, 8> PktOut_3B;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_3C_ID, 5 + MAX_CONTAINER_ITEMS * 20>
    PktOut_3C;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_4F_ID, 2> PktOut_4F;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_53_ID, 2> PktOut_53;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_54_ID, 12> PktOut_54;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_55_ID, 1> PktOut_55;
typedef PacketWriterDefs::PacketTemplate<Core::PKTBI_56_ID, 11> PktOut_56;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_65_ID, 4> PktOut_65;
typedef PacketWriterDefs::PacketTemplate<Core::PKTBI_66_ID, 0xFFFF> PktOut_66;
typedef PacketWriterDefs::PacketTemplate<Core::PKTBI_6C_ID, 19> PktOut_6C;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_6D_ID, 3> PktOut_6D;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_6E_ID, 14> PktOut_6E;
typedef PacketWriterDefs::PacketTemplate<Core::PKTBI_6F_ID, 47> PktOut_6F;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_70_ID, 28> PktOut_70;
typedef PacketWriterDefs::PacketTemplate<Core::PKTBI_72_ID, 5> PktOut_72;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_74_ID, 0xFFFF> PktOut_74;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_76_ID, 16> PktOut_76;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_77_ID, 17> PktOut_77;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_78_ID, 19 + ( 9 * Core::HIGHEST_LAYER ) + 4>
    PktOut_78;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_7C_ID, 2000> PktOut_7C;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_82_ID, 2> PktOut_82;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_88_ID, 66> PktOut_88;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_89_ID,
                                         7 + ( 5 * ( Core::NUM_LAYERS + 1 ) ) + 1>
    PktOut_89;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_8C_ID, 11> PktOut_8C;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_90_ID, 19> PktOut_90;
typedef PacketWriterDefs::PacketTemplate<Core::PKTBI_93_ID, 99> PktOut_93;
typedef PacketWriterDefs::PacketTemplate<Core::PKTBI_95_ID, 9> PktOut_95;
typedef PacketWriterDefs::PacketTemplate<Core::PKTBI_98_OUT_ID, 37> PktOut_98;
// HSA 30 old 26
typedef PacketWriterDefs::PacketTemplate<Core::PKTBI_99_ID, 30> PktOut_99;
typedef PacketWriterDefs::PacketTemplate<Core::PKTBI_9A_ID, 16> PktOut_9A;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_9E_ID, 0xFFFF> PktOut_9E;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_A1_ID, 9> PktOut_A1;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_A2_ID, 9> PktOut_A2;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_A3_ID, 9> PktOut_A3;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_A5_ID, URL_MAX_LEN + 4> PktOut_A5;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_A6_ID, 10010> PktOut_A6;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_A8_ID, 2000> PktOut_A8;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_A9_ID, 10000> PktOut_A9;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_AA_ID, 5> PktOut_AA;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_AB_ID, 531> PktOut_AB;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_AE_ID, ( SPEECH_MAX_LEN + 1 ) * 2 + 48>
    PktOut_AE;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_AF_ID, 13> PktOut_AF;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_B0_ID, 0xFFFF> PktOut_B0;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_B7_ID, 521> PktOut_B7;
typedef PacketWriterDefs::PacketTemplate<Core::PKTBI_B8_OUT_ID, 7 + 5 * ( SPEECH_MAX_LEN + 1 )>
    PktOut_B8;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_B9_ID, 5> PktOut_B9;
// HSA 10 old 6
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_BA_ID, 10> PktOut_BA;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_BC_ID, 3> PktOut_BC;

typedef PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3, Core::PKTBI_BF::TYPE_CLOSE_GUMP,
                                            5 + 8>
    PktOut_BF_Sub4;
typedef PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3, Core::PKTBI_BF::TYPE_PARTY_SYSTEM,
                                            0xFFFF>
    PktOut_BF_Sub6;
typedef PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3, Core::PKTBI_BF::TYPE_CURSOR_HUE,
                                            5 + 1>
    PktOut_BF_Sub8;
typedef PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3, Core::PKTBI_BF::TYPE_OBJECT_CACHE,
                                            5 + 8>
    PktOut_BF_Sub10;
typedef PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3,
                                            Core::PKTBI_BF::TYPE_DISPLAY_POPUP_MENU, 0xFFFF>
    PktOut_BF_Sub14;
typedef PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3, Core::PKTBI_BF::TYPE_CLOSE_WINDOW,
                                            5 + 8>
    PktOut_BF_Sub16;
typedef PacketWriterDefs::PacketTemplateSub<
    Core::PKTBI_BF_ID, 3, Core::PKTBI_BF::TYPE_ENABLE_MAP_DIFFS, 5 + 4 + MAX_NUMER_REALMS * 8>
    PktOut_BF_Sub18;
typedef PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3,
                                            Core::PKTBI_BF::TYPE_EXTENDED_STATS_OUT, 5 + 7>
    PktOut_BF_Sub19;
typedef PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3,
                                            Core::PKTBI_BF::TYPE_NEW_SPELLBOOK, 5 + 18>
    PktOut_BF_Sub1B;
typedef PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3,
                                            Core::PKTBI_BF::TYPE_CUSTOM_HOUSE_SHORT, 5 + 8>
    PktOut_BF_Sub1D;
typedef PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3,
                                            Core::PKTBI_BF::TYPE_ACTIVATE_CUSTOM_HOUSE_TOOL, 5 + 12>
    PktOut_BF_Sub20;
typedef PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3, Core::PKTBI_BF::TYPE_DAMAGE,
                                            5 + 6>
    PktOut_BF_Sub22;
typedef PacketWriterDefs::PacketTemplateSub<Core::PKTBI_BF_ID, 3,
                                            Core::PKTBI_BF::TYPE_CHARACTER_RACE_CHANGER, 5 + 2>
    PktOut_BF_Sub2A;

typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_C1_ID, 48 + ( SPEECH_MAX_LEN + 1 ) + 2>
    PktOut_C1;
typedef PacketWriterDefs::PacketTemplate<Core::PKTBI_C2_ID, 21> PktOut_C2;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_C7_ID, 49> PktOut_C7;
typedef PacketWriterDefs::PacketTemplate<Core::PKTBI_C8_ID, 2> PktOut_C8;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_CC_ID,
                                         49 + ( SPEECH_MAX_LEN + 1 ) * 2 + SPEECH_MAX_LEN + 1>
    PktOut_CC;
typedef PacketWriterDefs::PacketTemplate<Core::PKTBI_D6_OUT_ID, 0xFFFF> PktOut_D6;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_DC_ID, 9> PktOut_DC;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_DD_ID, 0xFFFF> PktOut_DD;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_DF_ID, 0xFFFF> PktOut_DF;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_E2_ID, 10> PktOut_E2;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_E3_ID, 77> PktOut_E3;
typedef PacketWriterDefs::PacketTemplateSub<Core::PKTBI_F0_ID, 1, Core::PKTBI_F0::QUERY_PARTY,
                                            0xFFFF>
    PktOut_F0_Sub01;
typedef PacketWriterDefs::PacketTemplateSub<Core::PKTBI_F0_ID, 1, Core::PKTBI_F0::QUERY_GUILD,
                                            0xFFFF>
    PktOut_F0_Sub02;
// HSA 26 old 24
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_F3_ID, 26> PktOut_F3;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_F5_ID, 21> PktOut_F5;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_F6_ID, 0xFFFF> PktOut_F6;
typedef PacketWriterDefs::PacketTemplate<Core::PKTOUT_F7_ID, 0xFFFF> PktOut_F7;
// Packet defs end
}  // namespace Network
}  // namespace Pol
#endif
