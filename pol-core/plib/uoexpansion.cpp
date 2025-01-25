#include "uoexpansion.h"
#include <array>
namespace Pol::Plib
{
constexpr u8 numExpansions = static_cast<u8>( ExpansionVersion::LastVersion ) + 1;
const std::array<std::string, numExpansions> ExpansionNames = { "T2A", "LBR", "AOS", "SE",
                                                                "ML",  "KR",  "SA",  "HSA" };

// make sure the flags have a defined value
// to prevent accidential changes
static_assert( B9Feature::DefaultT2A == (B9Feature)0x1 );
static_assert( B9Feature::DefaultLBR == (B9Feature)0x2 );
static_assert( B9Feature::DefaultAOS == (B9Feature)0x801B );
static_assert( B9Feature::DefaultSE == (B9Feature)0x805B );
static_assert( B9Feature::DefaultML == (B9Feature)0x80DB );
static_assert( B9Feature::DefaultKR == (B9Feature)0x86DB );
static_assert( B9Feature::DefaultSA == (B9Feature)0x187DF );
static_assert( B9Feature::DefaultHSA == (B9Feature)0x387DF );
static_assert( B9Feature::DefaultTOL == (B9Feature)0x7387DF );

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
    if ( str.find( ExpansionNames[e] ) != std::string::npos )
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
}  // namespace Pol::Plib
