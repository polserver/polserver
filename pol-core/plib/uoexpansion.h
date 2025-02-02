#pragma once

#include "../clib/rawtypes.h"
#include <fmt/format.h>
#include <string>

namespace Pol::Plib
{
enum class B9Feature : u32
{
  None = 0x0,
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
  JungleTiles = 0x100000,
  ShadowGuardTiles = 0x200000,
  TOL = 0x400000,


  DefaultT2A = T2A,                                                // 0x1
  DefaultLBR = Renaissance,                                        // 0x2
  DefaultAOS = DefaultT2A | DefaultLBR | LBR | AOS | LiveAcct,     // 0x801B
  DefaultSE = DefaultAOS | SE,                                     // 0x805B
  DefaultML = DefaultSE | ML,                                      // 0x80DB
  DefaultKR = DefaultML | CrystalShadowTiles | Splash10thAge,      // 0x86DB
  DefaultSA = DefaultKR | ThirdDawn | Splash8thAge | SA,           // 0x187DF
  DefaultHSA = DefaultSA | HSA | GothicTiles | RusticTiles,        // 0xF87DF
  DefaultTOL = DefaultHSA | JungleTiles | ShadowGuardTiles | TOL,  // 0x7F87DF
};


// minimal bitflag operators
inline constexpr B9Feature operator|( B9Feature a, B9Feature b )
{
  return static_cast<B9Feature>( static_cast<u32>( a ) | static_cast<u32>( b ) );
}
inline constexpr B9Feature operator&( B9Feature a, B9Feature b )
{
  return static_cast<B9Feature>( static_cast<u32>( a ) & static_cast<u32>( b ) );
}
inline constexpr B9Feature operator~( B9Feature a )
{
  return static_cast<B9Feature>( ~static_cast<u32>( a ) );
}
inline constexpr B9Feature& operator|=( B9Feature& a, B9Feature b )
{
  a = a | b;
  return a;
}
inline constexpr B9Feature& operator&=( B9Feature& a, B9Feature b )
{
  a = a & b;
  return a;
}

enum class A9Feature : u32
{
  None = 0x0,
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
// minimal bitflag operators
inline constexpr A9Feature operator&( A9Feature a, A9Feature b )
{
  return static_cast<A9Feature>( static_cast<u32>( a ) & static_cast<u32>( b ) );
}
inline constexpr A9Feature operator|( A9Feature a, A9Feature b )
{
  return static_cast<A9Feature>( static_cast<u32>( a ) | static_cast<u32>( b ) );
}
inline constexpr A9Feature& operator|=( A9Feature& a, A9Feature b )
{
  a = a | b;
  return a;
}

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
  TOL,
  LastVersion = TOL
};
std::string getExpansionName( ExpansionVersion x );
ExpansionVersion getExpansionVersion( const std::string& str );
B9Feature getDefaultExpansionFlag( ExpansionVersion x );

enum class FaceSupport : u8
{
  None,
  Basic,
  RolePlay
};

// hold in server
class ServerExpansion
{
public:
  ExpansionVersion Expansion() const { return expansion; };
  B9Feature extensionFlags() const { return ext_flags; };
  A9Feature featureFlags() const { return feature_flags; };
  u8 maxCharacterSlots() const { return char_slots; };

  bool supportsAOS() const { return supportsFeature( A9Feature::AOS ); };
  bool supportsFeature( A9Feature flag ) const
  {
    return ( feature_flags & flag ) != A9Feature::None;
  };
  FaceSupport faceSupport() const { return face_support; };


  ServerExpansion() = default;
  void updateFromPolCfg( u8 max_char_slots );
  void updateFromSSOpt( A9Feature feature, const std::string& version, u16 face_support );

private:
  ExpansionVersion expansion = ExpansionVersion::T2A;
  B9Feature ext_flags = B9Feature::DefaultT2A;
  A9Feature feature_flags = A9Feature::None;
  u8 char_slots = 5;
  FaceSupport face_support = FaceSupport::None;
};

// hold per account
class AccountExpansion
{
public:
  AccountExpansion() = default;
  AccountExpansion( const std::string& exp, B9Feature flag )
      : expansion( getExpansionVersion( exp ) ), ext_flags( flag ){};
  ExpansionVersion Expansion() const { return expansion; };
  B9Feature extensionFlags() const { return ext_flags; };
  A9Feature calculateFeatureFlags( const ServerExpansion& server ) const;
  B9Feature calculatedExtensionFlags( const ServerExpansion& server ) const;
  u8 getCharSlots( const ServerExpansion& server ) const;

private:
  ExpansionVersion expansion =
      ExpansionVersion::T2A;  // TODO needed? could save the flags or string depending if its
                              // default or not if flags can be changed
  B9Feature ext_flags = B9Feature::DefaultT2A;
};


inline auto format_as( B9Feature t )
{
  return fmt::underlying( t );
}
inline auto format_as( A9Feature t )
{
  return fmt::underlying( t );
}
inline auto format_as( ExpansionVersion t )
{
  return getExpansionName( t );
}
inline auto format_as( FaceSupport t )
{
  return fmt::underlying( t );
}
}  // namespace Pol::Plib
