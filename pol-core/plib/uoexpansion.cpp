#include "uoexpansion.h"
#include "../clib/clib.h"
#include <array>

namespace Pol::Plib
{
constexpr u8 numExpansions = static_cast<u8>( ExpansionVersion::LastVersion ) + 1;
const std::array<std::string, numExpansions> ExpansionNames = { "T2A", "LBR", "AOS", "SE", "ML",
                                                                "KR",  "SA",  "HSA", "TOL" };

// make sure the flags have a defined value
// to prevent accidential changes
static_assert( B9Feature::DefaultT2A == (B9Feature)0x1 );
static_assert( B9Feature::DefaultLBR == (B9Feature)0x2 );
static_assert( B9Feature::DefaultAOS == (B9Feature)0x801B );
static_assert( B9Feature::DefaultSE == (B9Feature)0x805B );
static_assert( B9Feature::DefaultML == (B9Feature)0x80DB );
static_assert( B9Feature::DefaultKR == (B9Feature)0x86DB );
static_assert( B9Feature::DefaultSA == (B9Feature)0x187DF );
static_assert( B9Feature::DefaultHSA == (B9Feature)0xF87DF );
static_assert( B9Feature::DefaultTOL == (B9Feature)0x7F87DF );

std::string getExpansionName( ExpansionVersion x )
{
  if ( x > ExpansionVersion::LastVersion )
    return "";
  return ExpansionNames[static_cast<u8>( x )];
}

ExpansionVersion getExpansionVersion( const std::string& str )
{
  for ( auto e = (u8)ExpansionVersion::T2A; e <= (u8)ExpansionVersion::LastVersion; ++e )
  {
    if ( str == ExpansionNames[e] )
      return static_cast<ExpansionVersion>( e );
  }
  return ExpansionVersion::T2A;
}

B9Feature getDefaultExpansionFlag( ExpansionVersion x )
{
  switch ( x )
  {
  case ExpansionVersion::T2A:
    return B9Feature::DefaultT2A;
  case ExpansionVersion::LBR:
    return B9Feature::DefaultLBR;
  case ExpansionVersion::AOS:
    return B9Feature::DefaultAOS;
  case ExpansionVersion::SE:
    return B9Feature::DefaultSE;
  case ExpansionVersion::ML:
    return B9Feature::DefaultML;
  case ExpansionVersion::KR:
    return B9Feature::DefaultKR;
  case ExpansionVersion::SA:
    return B9Feature::DefaultSA;
  case ExpansionVersion::HSA:
    return B9Feature::DefaultHSA;
  case ExpansionVersion::TOL:
    return B9Feature::DefaultTOL;
  }
  return B9Feature::DefaultT2A;
}

void ServerFeatures::updateFromSSOpt( A9Feature feature, const std::string& version,
                                      u16 facesupport )
{
  expansion = getExpansionVersion( version );
  ext_flags = getDefaultExpansionFlag( expansion );
  feature_flags = feature;
  Clib::sanitize_upperlimit<u16>( &facesupport, (u16)FaceSupport::RolePlay );
  face_support = (FaceSupport)facesupport;
}
void ServerFeatures::updateFromPolCfg( u8 max_char_slots )
{
  char_slots = max_char_slots;
}

std::string ServerFeatures::expansionName() const
{
  return getExpansionName( Expansion() );
}

A9Feature AccountExpansion::calculateFeatureFlags( const ServerFeatures& server ) const
{
  auto clientflag = server.featureFlags();
  clientflag |= A9Feature::UO3DClientType;  // Let UO3D (KR,SA) send 0xE1 packet

  auto char_slots = getCharSlots( server );

  if ( char_slots == 7 )
    clientflag |= A9Feature::Has7thSlot;  // 7th Character flag
  else if ( char_slots == 6 )
    clientflag |= A9Feature::Has6thSlot;  // 6th Character Flag
  else if ( char_slots == 1 )
    clientflag |= A9Feature::SingleCharacter |
                  A9Feature::LimitSlots;  // Only one character (SIEGE (0x04) + LIMIT_CHAR (0x10))
  return clientflag;
}

u8 AccountExpansion::getCharSlots( const ServerFeatures& server ) const
{
  u8 char_slots = server.maxCharacterSlots();
  // If more than 6 chars and no AOS, only send 5. Client is so boring sometimes...
  if ( char_slots >= 6 && ( Expansion() < ExpansionVersion::AOS ) )
    char_slots = 5;
  return char_slots;
}

B9Feature AccountExpansion::calculatedExtensionFlags( const ServerFeatures& server ) const
{
  auto clientflag = extensionFlags();
  // Change flag according to the number of CharacterSlots
  if ( Expansion() >= ExpansionVersion::AOS )
  {
    if ( server.maxCharacterSlots() == 7 )
    {
      clientflag |= B9Feature::Has7thSlot;  // 7th & 6th character flag (B9 Packet)
      clientflag &= ~B9Feature::ThirdDawn;  // Disable Third Dawn? TODO sounds wrong
    }
    else if ( server.maxCharacterSlots() == 6 )
    {
      clientflag |= B9Feature::Has6thSlot;  // 6th character flag (B9 Packet)
      clientflag &= ~B9Feature::ThirdDawn;  // TODO sounds wrong
    }
  }

  // Roleplay faces?
  if ( Expansion() >= ExpansionVersion::KR )
  {
    if ( server.faceSupport() == Plib::FaceSupport::RolePlay )
      clientflag |= B9Feature::KRFaces;
  }
  return clientflag;
}

std::string AccountExpansion::expansionName() const
{
  return getExpansionName( Expansion() );
}
}  // namespace Pol::Plib
