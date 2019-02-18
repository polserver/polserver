#ifndef PLIB_UOEXPANSION_H
#define PLIB_UOEXPANSION_H

#include "../clib/rawtypes.h"

namespace Pol
{
namespace Plib
{
enum class B9Feature : u32
{
  T2A = 0x01,          // Chats, regions (1.25.35)
  Renaissance = 0x02,  // Trammel/felucca (2.0.0)
  ThirdDawn = 0x04,    // Ilshenar, 3D client (3.0.0)
  LBR = 0x08,          // LBR skills, map (3.0.7b)
  AOS = 0x10,          // AOS skills, map, spells, fightbook (3.0.8z)
  Has6thSlot = 0x20,   // 6th character slot (4.0.3a)
  SE = 0x40,           // SE Features + housing tiles (4.0.5a)
  ML = 0x80,           // ML features: elven race, spells, skills + housing tiles

  Splash8thAge = 0x100,  // Splash screen 8th age (5.0.1a)

  CrystalShadowTiles = 0x200,  // Splash screen 9th age + crystal/shadow house tiles (5.0.5a)
  Splash9thAge = 0x200,        //    ^

  Splash10thAge = 0x400,  // Splash screen 10th age (6.0.3.0 ??)

  IncreasedStorage = 0x800,  // Increased house/bank storage (6.0.3.0 or earlier)

  Has7thSlot = 0x1000,  // 7th character slot (6.0.3.0 or earlier)
  KRFaces = 0x2000,     // ?? (6.0.0.0 was KR release)

  TrialAcct = 0x4000,
  LiveAcct = 0x8000,

  SA = 0x10000,           // Gargoyle race, spells, skills (7.0.0.0)
  HSA = 0x20000,          // HSA features (7.0.9.0)
  GothicTiles = 0x40000,  // Gothic housing tiles (7.0.9.0)
  RusticTiles = 0x80000,  // Rustic housing tiles (7.0.9.0)
};

enum class A9Feature : u32
{
  Unk1 = 0x01,             // Unknown, never sent by OSI
  ConfigReqLogout = 0x02,  // Send config/req logout (IGR? overwrite configuration button?)
  SingleCharacter = 0x04,  // Siege style 1 char/acct
  ContextMenus = 0x08,     // Enable NPC popup/context menus
  LimitSlots = 0x10,       // Limit character slots
  AOS = 0x20,              // AOS features (necromancer/paladin classes & tooltips) (3.0.8z)
  Has6thSlot = 0x40,       // 6th character slot (4.0.3a)

  SamuraiNinja = 0x80,  // Samurai and Ninja classes (4.0.5a)
  ElvenRace = 0x100,    // (4.0.11d)

  Unk2 = 0x200,            // Added with UOKR launch (6.0.0.0)
  UO3DClientType = 0x400,  // Send UO3D client type (client will send 0xE1 packet) (6.0.0.0)

  Unk3 = 0x800,  // Added between UOKR and UOSA launch

  Has7thSlot = 0x1000,  // 7th character slot, only 2D client (6.0.3.0 or earlier)

  Unk4 = 0x2000,  // Added with UOSA launch (7.0.0.0)

  NewMovement = 0x4000,            // New movement packets 0xF0 -> 0xF2 (7.0.0.0)
  UnlockNewFeluccaAreas = 0x8000,  // Unlock new felucca areas (factions map0x.mul and such)
                                   // (7.0.0.0 - SA or HSA, not sure)
};


enum class ExpansionVersion : u8
{
  T2A,
  LBR,
  AOS,
  SE,
  ML,
  KR,
  SA,
  HSA,

  LastVersion = HSA
};
const int numExpansions = static_cast<int>( ExpansionVersion::LastVersion ) + 1;
const char* getExpansionName( ExpansionVersion x );


class UOExpansion
{
public:
  virtual ~UOExpansion() = default;
  virtual bool hasFeature( A9Feature feature ) const = 0;
  virtual bool hasFeature( B9Feature feature ) const = 0;
  virtual u32 A9Flags() const = 0;
  virtual u32 B9Flags() const = 0;

  virtual ExpansionVersion version() const { return ExpansionVersion::T2A; }
  virtual int characterSlots() const { return 5; }
};

class ClientFeatures
{
  const UOExpansion& m_expansion;

public:
  ClientFeatures( UOExpansion& expansion ) : m_expansion( expansion ) {}
  bool hasCustomHousing() const { return supports( ExpansionVersion::AOS ); }
  bool hasSamuraiNinja() const { return m_expansion.hasFeature( A9Feature::SamuraiNinja ); }
  bool hasElvenRace() const { return m_expansion.hasFeature( A9Feature::ElvenRace ); }
  bool hasContextMenus() const { return m_expansion.hasFeature( A9Feature::ContextMenus ); }
  bool hasNewMovementPacket() const { return m_expansion.hasFeature( A9Feature::NewMovement ); }
  // Commonly used expansion
  bool supportsAOS() const { return supports( ExpansionVersion::AOS ); }
  bool supports( ExpansionVersion version ) const
  {
    ExpansionVersion myVersion = m_expansion.version();
    return myVersion >= version;
  }

  ClientFeatures( const ClientFeatures& ) = delete;
  ClientFeatures& operator=( const ClientFeatures& ) = delete;
};

class FlagExpansion : UOExpansion
{
protected:
  u32 m_A9Flag;
  u32 m_B9Flag;
  int m_slots;
  ExpansionVersion m_version;

public:
  FlagExpansion( u32 A9Flag, u32 B9Flag, int slots, ExpansionVersion version )
      : m_A9Flag( A9Flag ), m_B9Flag( B9Flag ), m_slots( slots ), m_version( version )
  {
  }

  virtual u32 A9Flags() const override { return m_A9Flag; }
  virtual u32 B9Flags() const override { return m_B9Flag; }
  virtual bool hasFeature( A9Feature x ) const override
  {
    return ( m_A9Flag & static_cast<u32>( x ) ) != 0;
  }
  virtual bool hasFeature( B9Feature x ) const override
  {
    return ( m_B9Flag & static_cast<u32>( x ) ) != 0;
  }

  virtual ExpansionVersion version() const override { return m_version; }
  virtual int characterSlots() const override { return m_slots; }
};
}  // namespace Plib
}  // namespace Pol
#endif
