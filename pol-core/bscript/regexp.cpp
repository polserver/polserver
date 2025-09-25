#include "regexp.h"
#include <stdexcept>

#include "bscript/berror.h"

namespace Pol::Bscript
{
BRegExp::BRegExp( boost::regex regex, boost::match_flag_type match_flags )
    : BObjectImp( OTRegExp ), regex_( std::move( regex ) ), match_flags_( match_flags )
{
}

BObjectImp* BRegExp::create( const std::string& pattern, const std::string& flags )
{
  boost::match_flag_type match_flags =
      boost::regex_constants::match_single_line | boost::regex_constants::format_first_only;

  boost::regex_constants::syntax_option_type flag = boost::regex_constants::ECMAScript;

  for ( const auto& ch : flags )
  {
    switch ( ch )
    {
    case 'i':
      flag |= boost::regex_constants::icase;
      break;
    case 'm':
      match_flags &= ~boost::regex_constants::match_single_line;
      break;
    case 'g':
      match_flags &= ~boost::regex_constants::format_first_only;
      break;
    default:
      return new BError( "Invalid flag character" );
    }
  }

  try
  {
    return new BRegExp( boost::regex( pattern, flag ), match_flags );
  }
  catch ( ... )
  {
    return new BError( "Invalid regular expression" );
  }
}

BRegExp::BRegExp( const BRegExp& i )
    : BObjectImp( OTRegExp ), regex_( i.regex_ ), match_flags_( i.match_flags_ )
{
}

BObjectImp* BRegExp::copy() const
{
  return new BRegExp( *this );
}

const char* BRegExp::typeOf() const
{
  return "RegExp";
}

u8 BRegExp::typeOfInt() const
{
  return OTRegExp;
}

std::string BRegExp::getStringRep() const
{
  return "<RegExp>";
}

size_t BRegExp::sizeEstimate() const
{
  return sizeof( BRegExp );
}

bool BRegExp::operator<( const BObjectImp& ) const
{
  return false;
}

bool BRegExp::isTrue() const
{
  return true;
}
}  // namespace Pol::Bscript
